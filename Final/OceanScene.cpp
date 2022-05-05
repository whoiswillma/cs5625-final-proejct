//
// Created by William Ma on 5/5/22.
//

#include <glm/gtx/transform.hpp>
#include "OceanScene.h"
#include "glm/glm.hpp"

OceanMesh::OceanMesh(int n, int m) {
    for (int j = 0; j < m; j++) {
        float y = (float) j / (float) (m - 1);

        for (int i = 0; i < n; i++) {
            float x = (float) i / (float) (n - 1);

            vertices.emplace_back(x, 0, y);
        }
    }

    for (uint32_t j = 0; j < m - 1; j++) {
        for (uint32_t i = 0; i < n - 1; i++) {
            uint32_t botLeft = j * n + i;
            uint32_t botRight = j * n + i + 1;
            uint32_t topLeft = (j + 1) * n + i;
            uint32_t topRight = (j + 1) * n + i + 1;

            // Push the lower-right triangle
            indices.push_back(botRight);
            indices.push_back(botLeft);
            indices.push_back(topRight);

            // Push the upper-left triangle
            indices.push_back(topLeft);
            indices.push_back(topRight);
            indices.push_back(botLeft);
        }
    }
}

OceanScene::OceanScene(glm::vec2 sizeMeters) : sizeMeters(sizeMeters) {
    const auto I = glm::identity<glm::mat4>();
    transform = glm::scale(I, glm::vec3(sizeMeters.x, 1, sizeMeters.y));
}
