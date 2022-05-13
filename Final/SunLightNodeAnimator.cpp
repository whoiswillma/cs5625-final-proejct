//
// Created by William Ma on 5/13/22.
//

#include "SunLightNodeAnimator.h"

#include "RTUtil/output.hpp"
#include <glm/gtx/transform.hpp>
#include <utility>

SunLightNodeAnimator::SunLightNodeAnimator(std::shared_ptr<Node> sunLightNode) : node(std::move(sunLightNode)) {
}

void SunLightNodeAnimator::update(float thetaSun) {
    node->transform = glm::rotate(thetaSun, glm::vec3(0, 0, 1));
}
