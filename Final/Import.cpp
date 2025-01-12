#include "Import.h"

#include <cassert>
#include <iostream>

#include "assimp/scene.h"

#include "RTUtil/conversions.hpp"
#include "RTUtil/output.hpp"

#include "MulUtil.hpp"

Mesh importMesh(aiMesh* aiMesh) {
    Mesh m;

    m.vertices.reserve(aiMesh->mNumVertices);
    m.normals.reserve(aiMesh->mNumVertices);
    m.boneIndices.reserve(aiMesh->mNumVertices);
    m.boneWeights.reserve(aiMesh->mNumVertices);

    for (int i = 0; i < aiMesh->mNumVertices; i++) {
        m.vertices.push_back(RTUtil::a2g(aiMesh->mVertices[i]));
        m.normals.push_back(RTUtil::a2g(aiMesh->mNormals[i]));
		if (aiMesh->mBones != 0) {
			m.uvcoordinates.push_back(glm::vec3(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y, aiMesh->mTextureCoords[0][i].z));
		}
		else {
			m.uvcoordinates.push_back(glm::vec3(-1, -1, -1));
		}
		
        m.boneIndices.emplace_back(-1, -1, -1, -1);
        m.boneWeights.emplace_back(0, 0, 0, 0);
    }

    for (int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        assert(face.mNumIndices == 3);
        for (int j = 0; j < 3; j++) {
            m.indices.push_back(face.mIndices[j]);
        }
    }

    m.materialIndex = aiMesh->mMaterialIndex;

    for (int i = 0; i < aiMesh->mNumBones; i++) {
        aiBone* aiBone = aiMesh->mBones[i];
        m.bones.emplace_back(std::string(aiBone->mName.C_Str()), RTUtil::a2g(aiBone->mOffsetMatrix));

        for (int j = 0; j < aiBone->mNumWeights; j++) {
            aiVertexWeight aiVertexWeight = aiBone->mWeights[j];

            for (int k = 0; k < 4; k++) {
                if (m.boneIndices[aiVertexWeight.mVertexId][k] == -1) {
                    m.boneIndices[aiVertexWeight.mVertexId][k] = i;
                    m.boneWeights[aiVertexWeight.mVertexId][k] = aiVertexWeight.mWeight;
                    break;
                }
            }
        }
    }

    std::cout << "Imported mesh " << aiMesh->mName.C_Str() << std::endl;
    std::cout << "\tnvertices = " << aiMesh->mNumVertices << std::endl;
    std::cout << "\tnbones = " << aiMesh->mNumBones << std::endl;

    return m;
}

std::vector<Mesh> importMeshes(const aiScene* aiScene) {
    std::vector<Mesh> meshes;
    for (int i = 0; i < aiScene->mNumMeshes; i++) {
        Mesh mesh = importMesh(aiScene->mMeshes[i]);
        meshes.push_back(mesh);
    }
    return meshes;
}

glm::mat4 nodeToWorldTransform(aiNode* node) {
    auto acc(glm::identity<glm::mat4>());
    for (aiNode* n = node; n != nullptr; n = n->mParent) {
        acc = RTUtil::a2g(n->mTransformation) * acc;
    }
    return acc;
}

std::shared_ptr<RTUtil::PerspectiveCamera> importCamera(const aiScene* aiScene) {
    std::shared_ptr<RTUtil::PerspectiveCamera> camera;

    if (aiScene->mNumCameras >= 1) {
        aiCamera *aiCamera = aiScene->mCameras[0];
        aiNode *cameraNode = aiScene->mRootNode->FindNode(aiCamera->mName);
        assert(cameraNode != nullptr);

        float fovy = 2.0f * glm::atan(glm::tan(aiCamera->mHorizontalFOV / 2.0f) / aiCamera->mAspect);
        glm::mat4 nodeToWorld = nodeToWorldTransform(cameraNode);

        std::cout << "Imported camera \"" << aiCamera->mName.C_Str() << "\"" << std::endl;

        camera = std::make_shared<RTUtil::PerspectiveCamera>(
                MulUtil::mulh(nodeToWorld, RTUtil::a2g(aiCamera->mPosition), 1),
                MulUtil::mulh(nodeToWorld, RTUtil::a2g(aiCamera->mLookAt), 1),
                glm::vec3(0, 1, 0),
                aiCamera->mAspect,
                aiCamera->mClipPlaneNear,
                aiCamera->mClipPlaneFar,
                fovy
        );
    } else {
        std::cout << "Using default camera" << std::endl;

        camera = std::make_shared<RTUtil::PerspectiveCamera>(
                glm::vec3(3, 4, 5),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0),
                1.0f,
                0.1f,
                1000.0f,
                glm::pi<float>() / 6
        );
    }

    return camera;
}

Material importMaterial(aiMaterial* aiMaterial) {
    Material material{};

    aiColor3D aiColor;
    aiMaterial->Get(AI_MATKEY_BASE_COLOR, aiColor);
    material.color = glm::vec3(aiColor.r, aiColor.g, aiColor.b);

    aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, material.roughnessFactor);

    return material;
}

std::vector<Material> importMaterials(const aiScene* aiScene) {
    std::vector<Material> materials;

    for (int i = 0; i < aiScene->mNumMaterials; i++) {
        materials.push_back(importMaterial(aiScene->mMaterials[i]));
    }

    return materials;
}

std::shared_ptr<Node> importNode(aiNode* aiNode) {
    std::shared_ptr<Node> node = std::make_shared<Node>();

    node->name = std::string(aiNode->mName.C_Str());
    node->transform = RTUtil::a2g(aiNode->mTransformation);

    for (int i = 0; i < aiNode->mNumMeshes; i++) {
        node->meshIndices.push_back(aiNode->mMeshes[i]);
    }

    for (int i = 0; i < aiNode->mNumChildren; i++) {
        std::shared_ptr<Node> child = importNode(aiNode->mChildren[i]);
        child->parent = node;

        node->children.push_back(child);
    }

    return node;
}

std::shared_ptr<Node> importRoot(const aiScene* aiScene) {
    return importNode(aiScene->mRootNode);
}

void importLights(
        const aiScene* aiScene,
        std::vector<std::shared_ptr<PointLight>>& pointLights,
        std::vector<std::shared_ptr<AreaLight>>& areaLights,
        std::vector<std::shared_ptr<AmbientLight>>& ambientLights
) {
    pointLights.clear();
    areaLights.clear();
    ambientLights.clear();

    for (int i = 0; i < aiScene->mNumLights; i++) {
        aiLight* aiLight = aiScene->mLights[i];

        std::string name(aiLight->mName.C_Str());
        float range, width, height;

        if (RTUtil::parseAmbientLight(name, range)) {
            auto light = std::make_shared<AmbientLight>();
            light->name = name;

            light->distance = range;
            light->radiance = RTUtil::a2g(aiLight->mColorAmbient);

            ambientLights.push_back(light);

            std::cout << "Imported ambient light \"" << name << "\"" << std::endl;
            std::cout << "\trange = " << light->distance << std::endl;
            std::cout << "\tradiance = " << light->radiance << std::endl;

        } else if (RTUtil::parseAreaLight(name, width, height)) {
            auto light = std::make_shared<AreaLight>();
            light->name = name;
            light->width = width;
            light->height = height;
            light->center = RTUtil::a2g(aiLight->mPosition);
            light->power = RTUtil::a2g(aiLight->mColorAmbient);

            areaLights.push_back(light);

            std::cout << "Imported area light \"" << name << "\"" <<  std::endl;
            std::cout << "\twidth = " << light->width << std::endl;
            std::cout << "\theight = " << light->height << std::endl;
            std::cout << "\tcenter(local) = " << light->center << std::endl;
            std::cout << "\tpower = " << light->power << std::endl;

        } else {
            auto light = std::make_shared<PointLight>();
            light->name = name;
            light->power = RTUtil::a2g(aiLight->mColorAmbient);
            light->position = RTUtil::a2g(aiLight->mPosition);

            pointLights.push_back(light);

            std::cout << "Imported point light \"" << name << "\"" << std::endl;
            std::cout << "\tpower = " << light->power << std::endl;
            std::cout << "\tposition(local) = " << light->position << std::endl;
        }
    }

    std::cout << "Imported " << aiScene->mNumLights << " lights" << std::endl;
}

Channel importChannel(const aiNodeAnim* aiChannel) {
    Channel channel;

    channel.nodeName = std::string(aiChannel->mNodeName.C_Str());

    for (int i = 0; i < aiChannel->mNumPositionKeys; i++) {
        aiVectorKey key = aiChannel->mPositionKeys[i];
        channel.translation.insert({ key.mTime, RTUtil::a2g(key.mValue) });
    }

    for (int i = 0; i < aiChannel->mNumRotationKeys; i++) {
        aiQuatKey key = aiChannel->mRotationKeys[i];
        channel.rotation.insert({ key.mTime, RTUtil::a2g(key.mValue) });
    }

    for (int i = 0; i < aiChannel->mNumScalingKeys; i++) {
        aiVectorKey key = aiChannel->mScalingKeys[i];
        channel.scale.insert({ key.mTime, RTUtil::a2g(key.mValue) });
    }

    return channel;
}

Animation importAnimation(const aiAnimation* aiAnimation) {
    Animation animation;

    animation.duration = aiAnimation->mDuration;
    animation.ticksPerSecond = aiAnimation->mTicksPerSecond;

    for (int i = 0; i < aiAnimation->mNumChannels; i++) {
        animation.channels.push_back(importChannel(aiAnimation->mChannels[i]));
    }

    return animation;
}

void importAnimations(const aiScene* aiScene, std::vector<Animation>& animations) {
    for (int i = 0; i < aiScene->mNumAnimations; i++) {
        animations.push_back(importAnimation(aiScene->mAnimations[i]));
    }

    std::cout << "Imported " << animations.size() << " animations" << std::endl;
}

void dumpNodeHierarchy(const std::shared_ptr<Node>& node, int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << " ";
    }
    std::cout << node->name << " " << node->transform << std::endl;

    for (const auto& child : node->children) {
        dumpNodeHierarchy(child, indent + 1);
    }
}

std::shared_ptr<Scene> importScene(const aiScene* aiScene) {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    scene->meshes = importMeshes(aiScene);
    scene->camera = importCamera(aiScene);
    scene->materials = importMaterials(aiScene);
    scene->root = importRoot(aiScene);
    importLights(aiScene, scene->pointLights, scene->areaLights, scene->ambientLights);
    importAnimations(aiScene, scene->animations);
    dumpNodeHierarchy(scene->root, 0);
    return scene;
}


