//
// Created by William Ma on 5/5/22.
//

#include <glm/gtx/transform.hpp>
#include "OceanScene.h"
#include "glm/glm.hpp"
#include "MulUtil.hpp"
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
    }),
    upwelling(0.02, 0.03, 0.07) {
}

glm::mat4 OceanScene::transform(glm::vec2 gridLocation) const {
    return glm::scale(glm::vec3(sizeMeters.x, 1.0f, sizeMeters.y))
        * glm::translate(glm::vec3(-0.5 + gridLocation.x, 0, -0.5 + gridLocation.y));
}

bool OceanScene::gridLocationIsVisible(glm::vec2 gridLocation, glm::mat4 mViewProj) const {
    glm::mat4 gridToNDC = mViewProj * transform(gridLocation);

    // TODO: improve this with triangle/cube intersection tests

    const int n = 5;
    std::vector<glm::vec3> testPoints;
    for (int i = 0; i < n; i++) {
        float x = (float) i / (float) (n - 1);
        for (int j = 0; j < n; j++) {
            float y = (float) j / (float) (n - 1);
            testPoints.push_back(MulUtil::mulh(gridToNDC, glm::vec3(x, 0, y), 1));
        }
    }
    bool anyPointVisible = false;

    for (auto & point : testPoints) {
        bool thisPointVisible = true;

        for (int j = 0; j < 3; j++) {
            if (!(-1.0f <= point[j] && point[j] <= 1.0f)) {
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


std::vector<glm::vec2> OceanScene::visibleGridLocations(glm::mat4 mViewProj, int visibleLimit, int searchRadius) {
    std::vector<glm::vec2> visibleGridLocations;

    std::deque<glm::vec2> queue;
    {   // Cast a bunch of rays into the scene and use the nearest grid location as the starting point of the search.
        glm::mat4 mNdcToGrid = glm::inverse(mViewProj * transform());

        for (int x = 0; x <= 0; x++) {
            for (int y = 0; y <= 0; y++) {
                glm::vec3 near = MulUtil::mulh(mNdcToGrid, glm::vec3(x, y, -1), 1);
                glm::vec3 far = MulUtil::mulh(mNdcToGrid, glm::vec3(x, y, 1), 1);

                const float a = far.y / (far.y - near.y);
                glm::vec3 intersection = a * near + (1 - a) * far;
                glm::vec2 gridPoint = glm::round(glm::vec2(intersection.x, intersection.z));
                queue.push_back(gridPoint);
            }
        }
    }

    std::unordered_set<glm::vec2, Vec2Hasher> visited;

    int searchLimit = (2 * searchRadius + 1) * (2 * searchRadius + 1);
    while (
            visited.size() < searchLimit
            && !queue.empty()
            && (visibleLimit == -1 || visibleGridLocations.size() < visibleLimit)
    ) {
        glm::vec2 gridPoint = queue.front();
        queue.pop_front();

        if (visited.find(gridPoint) != visited.end()) {
            continue;
        }
        visited.insert(gridPoint);

        bool visible = gridLocationIsVisible(gridPoint, mViewProj);
        if (visible) {
            visibleGridLocations.push_back(gridPoint);
            for (auto & gridLocation : adjacentGridLocations(gridPoint)) {
                queue.push_back(gridLocation);
            }
        }
    }

    return visibleGridLocations;
}
