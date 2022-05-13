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

    explicit OceanMesh(int n, int m);
};

struct OceanScene {
    OceanMesh mesh;

    const tessendorf::config config;
    const tessendorf::array2d<std::complex<float>> tessendorfIv;

    const glm::ivec2 gridSize;
    const glm::vec2 sizeMeters;
    const glm::vec3 upwelling;

    OceanScene(glm::vec2 sizeMeters, glm::ivec2 gridSize);

    glm::mat4 transform(glm::vec2 gridLocation = glm::vec2(0, 0)) const;
    std::vector<glm::vec2> visibleGridLocations(
            glm::mat4 mViewProj,
            int visibleLimit = 1000,
            int searchRadius = 100
    );

private:
    bool gridLocationIsVisible(glm::vec2 gridLocation, glm::mat4 mViewProj) const;
};

#endif //CS5625_OCEANSCENE_H
