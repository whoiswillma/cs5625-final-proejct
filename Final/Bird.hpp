//
// Created by Adith Ramachandran on 5/8/22.
//

#pragma once

#include <string>
#include <random>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Scene.h"

struct Wall
{
    Wall(glm::vec3 point, glm::vec3 normal)
    {
        this->point = point;
        this->normal = normal;
    }
    glm::vec3 point;
    glm::vec3 normal;
};

struct Bird
{
    Bird(std::shared_ptr<Node> birdNode);

    static bool is_bird(const std::string& nodeName) { return nodeName.find("Bird") != std::string::npos; };
    static std::vector<Wall> walls;
    void update_self(glm::vec3 deltaV);

    glm::vec3 position;
    float initVelocity;
    glm::vec3 velocity;
    glm::mat4 rotScale;
    std::shared_ptr<Node> nodePtr;
};
