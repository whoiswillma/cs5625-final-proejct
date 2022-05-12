//
// Created by William Ma on 5/12/22.
//

#include "BoatNodeAnimator.h"
#include "MulUtil.hpp"
#include <glm/gtx/transform.hpp>
#include "glm/gtx/quaternion.hpp"
#include "RTUtil/output.hpp"

BoatNodeAnimator::BoatNodeAnimator(std::shared_ptr<Node> boatNode) {
    node = boatNode;
}

void BoatNodeAnimator::update(
        tessendorf::array2d<float> displacementMap,
        tessendorf::array2d<float> gradXMap,
        tessendorf::array2d<float> gradZMap,
        glm::mat4 oceanTransform
) {

    glm::vec3 positionAcc;
    glm::vec3 normalAcc;
    float sigma = 1;
    int r = 5;
    int d = 2 * r + 1;
    for (int dx = -d; dx <= d; dx++) {
        for (int dy = -d; dy <= d; dy++) {
            glm::vec3 position(
                    0,
                    displacementMap.get(displacementMap.size_x / 2 + dx, displacementMap.size_y / 2 + dy),
                    0
            );

            glm::vec3 normal = glm::vec3(
                    gradXMap.get(gradXMap.size_x / 2 + dx, gradXMap.size_y / 2 + dy),
                    1.0f,
                    gradZMap.get(gradZMap.size_x / 2 + dx, gradZMap.size_y / 2 + dy)
            );
            glm::vec4 normal4 = glm::vec4(normal, 0.0);
            normal4 = glm::transpose(glm::inverse(oceanTransform)) * normal4;
            normal = glm::normalize(glm::vec3(normal4.x, normal4.y, normal4.z));

            float weight = 1.0f / (2 * M_PI * sigma * sigma) * exp(-(dx * dx + dy * dy) / (2 * sigma * sigma));
            normalAcc += weight * normal;
            positionAcc += weight * position;
        }
    }

    normalAcc = glm::normalize(normalAcc);

    node->transform =
            glm::translate(positionAcc)
            * glm::mat4_cast(glm::rotation(glm::vec3(0, 0, 1), normalAcc))
            * glm::scale(glm::vec3(0.5));
}


