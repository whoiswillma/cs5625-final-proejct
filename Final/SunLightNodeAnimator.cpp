//
// Created by William Ma on 5/13/22.
//

#include "SunLightNodeAnimator.h"

#include "RTUtil/output.hpp"
#include "RTUtil/Sky.hpp"
#include <glm/gtx/transform.hpp>
#include <utility>

SunLightNodeAnimator::SunLightNodeAnimator(
        std::shared_ptr<PointLight> light,
        std::shared_ptr<Node> node
) : node(std::move(node)),
    light(std::move(light)) {
}

void SunLightNodeAnimator::update(float thetaSun, float turbidity) {
    node->transform = glm::rotate(thetaSun, glm::vec3(0, 0, 1));
}
