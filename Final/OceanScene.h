//
// Created by William Ma on 5/5/22.
//

#ifndef CS5625_OCEANSCENE_H
#define CS5625_OCEANSCENE_H

#include <vector>
#include <glm/glm.hpp>
#include <complex>
#include "pocketfft_hdronly.h"
#include "Tessendorf.h"

struct OceanMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<uint32_t> indices;

    explicit OceanMesh(int n = 128, int m = 128);
};

struct OceanScene {
    glm::mat4 transform;
    OceanMesh mesh;

    const tessendorf::array2d<std::complex<float>> tessendorfIv;

    const glm::ivec2 gridSize;
    const glm::vec2 sizeMeters;

    OceanScene(glm::vec2 sizeMeters, glm::ivec2 gridSize);
};

#endif //CS5625_OCEANSCENE_H
