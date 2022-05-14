#ifndef CS5625_SCENE_H
#define CS5625_SCENE_H

#include <memory>
#include <utility>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "RTUtil/Camera.hpp"
#include <functional>

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
};

struct PointLight {
    std::string name;
    glm::vec3 position;
    glm::vec3 power;
};

struct Node {
    std::string name;
    glm::mat4 transform;
    std::vector<unsigned int> meshIndices;
    std::vector<std::shared_ptr<Node>> children;
    std::shared_ptr<Node> parent;

    glm::mat4 getTransformTo(const std::shared_ptr<Node>& other);
};

struct Mesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
	std::vector<glm::vec3> uvcoordinates;
    std::vector<uint32_t> indices;
    uint32_t materialIndex;

    std::vector<std::pair<std::string, glm::mat4>> bones;
    std::vector<glm::vec4> boneWeights;
    std::vector<glm::ivec4> boneIndices;
};

struct Channel {
    std::string nodeName;
    std::map<double, glm::vec3> translation;
    std::map<double, glm::quat> rotation;
    std::map<double, glm::vec3> scale;
};

struct Animation {
    double ticksPerSecond;
    std::vector<Channel> channels;
    double duration;
};

struct Scene {
    std::vector<Mesh> meshes;
    std::shared_ptr<RTUtil::PerspectiveCamera> camera;
    std::vector<Material> materials;
    std::vector<std::shared_ptr<PointLight>> pointLights;
    std::vector<std::shared_ptr<AreaLight>> areaLights;
    std::vector<std::shared_ptr<AmbientLight>> ambientLights;
    std::shared_ptr<Node> root;
    std::vector<Animation> animations;

    void animate(double time, unsigned int animationIdx = 0);

    std::shared_ptr<Node> findNode(const std::string& name);

private:
    std::map<std::string, std::shared_ptr<Node>> nameToNode;
};
#endif //CS5625_SCENE_H