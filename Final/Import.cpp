#include "Import.h"

#include <cassert>
#include <iostream>

#include "assimp/scene.h"

#include "RTUtil/conversions.hpp"
#include "RTUtil/output.hpp"

#include "MulUtil.hpp"

Mesh importMesh(aiMesh* aiMesh) {
    Mesh m;

    for (int i = 0; i < aiMesh->mNumVertices; i++) {
        m.vertices.push_back(RTUtil::a2g(aiMesh->mVertices[i]));
        m.normals.push_back(RTUtil::a2g(aiMesh->mNormals[i]));
    }

    for (int i = 0; i < aiMesh->mNumFaces; i++) {
        aiFace face = aiMesh->mFaces[i];
        assert(face.mNumIndices == 3);
        for (int j = 0; j < 3; j++) {
            m.indices.push_back(face.mIndices[j]);
        }
    }

    m.materialIndex = aiMesh->mMaterialIndex;

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
    if (aiScene->mNumCameras >= 1) {
        aiCamera* camera = aiScene->mCameras[0];
        aiNode* cameraNode = aiScene->mRootNode->FindNode(camera->mName);
        assert(cameraNode != nullptr);

        float fovy = 2.0f * glm::atan(glm::tan(camera->mHorizontalFOV / 2.0f) / camera->mAspect);
        glm::mat4 nodeToWorld = nodeToWorldTransform(cameraNode);

        std::cout << "Imported camera \"" << camera->mName.C_Str() << "\"" << std::endl;

        return std::make_shared<RTUtil::PerspectiveCamera>(
                MulUtil::mulh(nodeToWorld, RTUtil::a2g(camera->mPosition), 1),
                MulUtil::mulh(nodeToWorld, RTUtil::a2g(camera->mLookAt), 1),
                MulUtil::mulh(nodeToWorld, RTUtil::a2g(camera->mUp), 0),
                camera->mAspect,
                camera->mClipPlaneNear,
                camera->mClipPlaneFar,
                fovy
        );

    } else {
        std::cout << "Using default camera" << std::endl;

        return std::make_shared<RTUtil::PerspectiveCamera>(
                glm::vec3(3, 4, 5),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0),
                1.0f,
                0.1f,
                1000.0f,
                glm::pi<float>() / 6
        );
    }
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
        std::vector<PointLight>& pointLights,
        std::vector<AreaLight>& areaLights,
        std::vector<AmbientLight>& ambientLights
) {

    for (int i = 0; i < aiScene->mNumLights; i++) {
        aiLight* aiLight = aiScene->mLights[i];

        std::string name(aiLight->mName.C_Str());
        float range, width, height;

        if (RTUtil::parseAmbientLight(name, range)) {
            AmbientLight light;
            light.name = name;

            light.distance = range;
            light.radiance = RTUtil::a2g(aiLight->mColorAmbient);

            ambientLights.push_back(light);

            std::cout << "Imported ambient light \"" << name << "\"" << std::endl;
            std::cout << "\trange = " << light.distance << std::endl;
            std::cout << "\tradiance = " << light.radiance << std::endl;

        } else if (RTUtil::parseAreaLight(name, width, height)) {
            AreaLight light;
            light.name = name;
            light.width = width;
            light.height = height;
            light.center = RTUtil::a2g(aiLight->mPosition);
            light.power = RTUtil::a2g(aiLight->mColorAmbient);

            aiNode* aiNode = aiScene->mRootNode->FindNode(aiLight->mName);
            light.nodeToWorld = nodeToWorldTransform(aiNode);

            areaLights.push_back(light);

            std::cout << "Imported area light \"" << name << "\"" <<  std::endl;
            std::cout << "\twidth = " << light.width << std::endl;
            std::cout << "\theight = " << light.height << std::endl;
            std::cout << "\tcenter(local) = " << light.center << std::endl;
            std::cout << "\tpower = " << light.power << std::endl;

        } else {
            PointLight light;
            light.name = name;
            light.power = RTUtil::a2g(aiLight->mColorAmbient);
            light.position = RTUtil::a2g(aiLight->mPosition);

            aiNode* aiNode = aiScene->mRootNode->FindNode(aiLight->mName);
            light.nodeToWorld = nodeToWorldTransform(aiNode);

            pointLights.push_back(light);

            std::cout << "Imported point light \"" << name << "\"" << std::endl;
            std::cout << "\tpower = " << light.power << std::endl;
            std::cout << "\tposition(local) = " << light.position << std::endl;
        }
    }

    std::cout << "Imported " << aiScene->mNumLights << " lights" << std::endl;
}

std::shared_ptr<Scene> importScene(const aiScene* aiScene) {
    std::shared_ptr<Scene> scene(new Scene());
    scene->meshes = importMeshes(aiScene);
    scene->camera = importCamera(aiScene);
    scene->materials = importMaterials(aiScene);
    scene->root = importRoot(aiScene);
    importLights(aiScene, scene->pointLights, scene->areaLights, scene->ambientLights);
    return scene;
}
