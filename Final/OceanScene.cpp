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
            texCoords.emplace_back(x, y);
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

OceanScene::OceanScene(glm::vec2 sizeMeters, glm::ivec2 gridSize) :
	mesh(gridSize.x, gridSize.y),
	sizeMeters(sizeMeters),
	gridSize(gridSize),
	tessendorfIv(tessendorf::sample_initialization_vector(gridSize, std::default_random_engine())),
	config({
		10.0f,
		sizeMeters,
		12.0f,
		glm::normalize(glm::vec2(1.0, 0.2)),
		3.0f
		}) {
	const auto I = glm::identity<glm::mat4>();
	transform = glm::translate(glm::vec3(0, -1, 0))
		* glm::scale(glm::vec3(sizeMeters.x, 1.0f, sizeMeters.y))
		* glm::translate(glm::vec3(-0.5, 0, -0.5));
}