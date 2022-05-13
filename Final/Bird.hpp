//
// Created by Adith Ramachandran on 5/8/22.
//

#pragma once

#include <string>
#include <random>
#include <map>
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

struct Wind
{
    Wind() {
        std::random_device rd;
        std::mt19937_64 mt(rd());
        std::uniform_real_distribution<float> dist(-0.25, 0.25);
        for (int i = 0; i < 10; i++) {
            this->windDirs.insert(std::make_pair(i, glm::vec3 {
                dist(mt), dist(mt), dist(mt)
            }));
        }
    }

    glm::vec3 get_wind_dir(double time) {
        double mapSize = this->windDirs.size();
        return glm::mix(
                this->windDirs.lower_bound(std::fmod(time, mapSize))->second,
                this->windDirs.lower_bound(std::fmod(time + 1, mapSize))->second,
                std::fmod(time, 1)
                );
    }
    std::map<int, glm::vec3> windDirs;
};

struct Bird
{
    Bird(std::shared_ptr<Node> birdNode);

    static bool is_bird(const std::string& nodeName) { return nodeName.find("Bird") != std::string::npos; };
    static std::vector<Wall> walls;
    static Wind wind;
    void update_self(glm::vec3 deltaV);

    glm::vec3 position;
    float initVelocity;
    glm::vec3 velocity;
    glm::mat4 rotScale;
    std::shared_ptr<Node> nodePtr;
};
