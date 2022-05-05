//
// Created by William Ma on 5/5/22.
//

#ifndef CS5625_OCEANSCENE_H
#define CS5625_OCEANSCENE_H

#include <vector>
#include <glm/glm.hpp>
#include "pocketfft_hdronly.h"

struct OceanMesh {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;

    explicit OceanMesh(int n = 128, int m = 128);
};

struct OceanScene {
    glm::mat4 transform;
    OceanMesh mesh;

    glm::vec2 sizeMeters;

    explicit OceanScene(
            glm::vec2 sizeMeters
    );
};

#endif //CS5625_OCEANSCENE_H
