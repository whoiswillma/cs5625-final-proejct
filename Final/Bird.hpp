//
// Created by Adith Ramachandran on 5/8/22.
//

#pragma once

#include <string>
#include <random>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Scene.h"

struct Bird
{
    Bird(std::shared_ptr<Node> birdNode);

    static bool isBird(const std::string& nodeName) { return nodeName.find("Bird") != std::string::npos; };

    glm::vec3 position;
    glm::qua<float> orientation;
    std::shared_ptr<Node> nodePtr;
};
