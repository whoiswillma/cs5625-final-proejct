#ifndef CS5625_SCENE_H
#define CS5625_SCENE_H

#include <memory>
#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "RTUtil/Camera.hpp"

#define MESH_IDX_OCEAN (unsigned int) -1

struct Material {
    glm::vec3 color;
    float roughnessFactor;
};

struct AmbientLight {
    std::string name;
    glm::vec3 radiance;
    float distance;
};

struct AreaLight {
    std::string name;
    float width;
    float height;
    glm::vec3 center;
    glm::vec3 power;
    glm::mat4 nodeToWorld;
};

struct PointLight {
    std::string name;
    glm::vec3 position;
    glm::mat4 nodeToWorld;
    glm::vec3 power;
};

struct Node {
    glm::mat4 transform;
    std::vector<unsigned int> meshIndices;
    std::vector<std::shared_ptr<Node>> children;
    std::shared_ptr<Node> parent;

    Node(
            glm::mat4 transform,
            std::vector<unsigned int> meshIndices,
            std::vector<std::shared_ptr<Node>> children,
            std::shared_ptr<Node> parent
    ) : transform(transform),
        meshIndices(std::move(meshIndices)),
        children(std::move(children)),
        parent(std::move(parent)) {
    }

    glm::mat4 getTransformTo(const std::shared_ptr<Node>& other);
};

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;
};

struct OceanMesh {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

    explicit OceanMesh(int n = 100, int m = 100);
};

struct Scene {
    std::vector<Mesh> meshes;
    std::shared_ptr<RTUtil::PerspectiveCamera> camera;
    std::vector<Material> materials;
    std::vector<PointLight> pointLights;
    std::vector<AreaLight> areaLights;
    std::vector<AmbientLight> ambientLights;
    std::shared_ptr<Node> root;

    OceanMesh oceanMesh;

    Scene();
};

#endif //CS5625_SCENE_H
