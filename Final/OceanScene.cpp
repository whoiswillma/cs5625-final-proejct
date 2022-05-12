//
// Created by William Ma on 5/5/22.
//

#include <glm/gtx/transform.hpp>
#include "OceanScene.h"
#include "glm/glm.hpp"
#include "MulUtil.hpp"
#include "RTUtil/output.hpp"
#include <unordered_set>

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
}

glm::mat4 OceanScene::transform(glm::vec2 gridLocation) const {
    return glm::scale(glm::vec3(sizeMeters.x, 1.0f, sizeMeters.y))
        * glm::translate(glm::vec3(-0.5 + gridLocation.x, 0, -0.5 + gridLocation.y));
}

bool OceanScene::gridLocationIsVisible(glm::vec2 gridLocation, glm::mat4 mWorldToNDC, float bias) const {
    glm::mat4 gridToNDC = mWorldToNDC * OceanScene::transform(gridLocation);
    glm::vec3 testPoints[9] = {
            MulUtil::mulh(gridToNDC, glm::vec3(0, 0, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(0, 0, 1), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(1, 0, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(1, 0, 1), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(0.5, 0.5, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(0.5, 0, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(-0.5, 0, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(0, 0.5, 0), 1),
            MulUtil::mulh(gridToNDC, glm::vec3(0, -0.5, 0), 1),
    };

    bool anyPointVisible = false;

    for (auto & point : testPoints) {
        bool thisPointVisible = true;

        for (int j = 0; j < 3; j++) {
            if (!(-1.0f - bias <= point[j] && point[j] <= 1.0f + bias)) {
                thisPointVisible = false;
                break;
            }
        }

        if (thisPointVisible) {
            anyPointVisible = true;
            break;
        }
    }

    return anyPointVisible;
}

std::vector<glm::vec2> adjacentGridLocations(glm::vec2 gridLocation) {
    return {
        gridLocation + glm::vec2(1, 0),
        gridLocation + glm::vec2(1, 1),
        gridLocation + glm::vec2(0, 1),
        gridLocation + glm::vec2(-1, 1),
        gridLocation + glm::vec2(-1, 0),
        gridLocation + glm::vec2(-1, -1),
        gridLocation + glm::vec2(0, -1),
        gridLocation + glm::vec2(1, -1),
    };
}


struct Vec2Hasher {
public:
    size_t operator()(glm::vec2 vec) const {
        std::hash<float> hasher;

        // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
        size_t seed = hasher(vec.x);
        return seed ^ (hasher(vec.y) + 0x9e3779b9 + (seed<<6) + (seed>>2));
    }
};


std::vector<glm::vec2> OceanScene::visibleGridLocations(glm::mat4 mWorldToNDC, int softLimit, int searchRadius) {
    std::vector<glm::vec2> visibleGridLocations;

    std::deque<glm::vec2> queue;
    queue.emplace_back(0, 0);

    std::unordered_set<glm::vec2, Vec2Hasher> visited;

    int phase0Time = 0;
    int phase = 0;
    int searchLimit = (2 * searchRadius + 1) * (2 * searchRadius + 1);
    while (
            visited.size() < searchLimit
            && !queue.empty()
            && (softLimit == -1 || visibleGridLocations.size() < softLimit)
    ) {
        glm::vec2 gridPoint = queue.front();
        queue.pop_front();

        if (visited.find(gridPoint) != visited.end()) {
            continue;
        }
        visited.insert(gridPoint);

        bool visible = gridLocationIsVisible(gridPoint, mWorldToNDC, 1e-5);
        if (visible) {
            phase = 1;
            visibleGridLocations.push_back(gridPoint);
        }

        if ((phase == 0) || (phase == 1 && visible)) {
            for (auto & gridLocation : adjacentGridLocations(gridPoint)) {
                queue.push_back(gridLocation);
            }
        }

        if (phase == 0) {
            phase0Time++;
        }
    }

    std::cout << "ratio: " << (float) visibleGridLocations.size() / (float) visited.size() << std::endl;
    std::cout << visibleGridLocations.size() << ", " << phase0Time << ", " << std::endl;

    return visibleGridLocations;
}
