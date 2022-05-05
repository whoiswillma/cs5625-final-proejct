#include "Scene.h"

#include <memory>

glm::mat4 Node::getTransformTo(const std::shared_ptr<Node>& other) {
    if (other == nullptr) {
        auto acc = glm::identity<glm::mat4>();

        for (Node* node = this; node != nullptr; node = node->parent.get()) {
            acc = node->transform * acc;
        }

        return acc;

    } else {
        return glm::inverse(other->getTransformTo(nullptr)) * getTransformTo(nullptr);
    }
}

Scene::Scene() {
    root = std::make_shared<Node>(
        glm::identity<glm::mat4>(),
        std::vector<unsigned int>(),
        std::vector<std::shared_ptr<Node>>(),
        nullptr
    );

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

OceanMesh::OceanMesh(int n, int m) {
    for (int j = 0; j < m; j++) {
        float y = (float) j / (float) (m - 1);

        for (int i = 0; i < n; i++) {
            float x = (float) i / (float) (n - 1);

            vertices.emplace_back(x, y, 0);
        }
    }

    for (uint32_t j = 0; j < m - 1; j++) {
        for (uint32_t i = 0; i < n - 1; i++) {
            uint32_t botLeft = j * n + i;
            uint32_t botRight = j * n + i + 1;
            uint32_t topLeft = (j + 1) * n + i;
            uint32_t topRight = (j + 1) * n + i + 1;

            // Push the lower-right triangle
            indices.push_back(botLeft);
            indices.push_back(botRight);
            indices.push_back(topRight);

            // Push the upper-left triangle
            indices.push_back(topRight);
            indices.push_back(topLeft);
            indices.push_back(botLeft);
        }
    }
}
