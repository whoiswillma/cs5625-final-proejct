//
// Created by Adith Ramachandran on 5/8/22.
//

#include "Bird.hpp"

const double bound = 10.0;
const float randomVelocity = 0.05;
std::vector<Wall> Bird::walls {
        Wall{ glm::vec3 {  bound, bound,  0 }, glm::normalize(glm::vec3{  -bound, 0,  0 }) },
        Wall{ glm::vec3 { -bound, bound,  0 }, glm::normalize(glm::vec3{   bound, 0,  0 }) },
        Wall{ glm::vec3 {  0,   bound,  bound }, glm::normalize(glm::vec3{ 0,   0, -bound }) },
        Wall{ glm::vec3 {  0,   bound, -bound }, glm::normalize(glm::vec3{ 0,   0,  bound }) }
};

Bird::Bird(std::shared_ptr<Node> birdNode) {
    std::random_device rd;
    std::mt19937_64 mt(rd());
    std::uniform_int_distribution<int> dist_int(-7, 7);
    std::uniform_real_distribution<float> dist_real(-randomVelocity, randomVelocity);

    this->nodePtr = birdNode;
    this->position = glm::vec3{ dist_int(mt), 7.5, dist_int(mt) };
    this->velocity = glm::vec3{ dist_real(mt), 0, dist_real(mt) };
    std::cout << this->velocity.x << " " << this->velocity.y << " " << this->velocity.z << std::endl;
    this->scale = glm::scale(glm::mat4(1), glm::vec3{
        glm::length(glm::vec3(this->nodePtr->transform[0])),
        glm::length(glm::vec3(this->nodePtr->transform[1])),
        glm::length(glm::vec3(this->nodePtr->transform[2]))
    });

    update_self();
}

void Bird::update_self() const {
    this->nodePtr->transform =
            glm::translate(glm::mat4(1), this->position) *
            glm::mat4_cast(glm::quatLookAt(this->velocity, glm::vec3{ 0, 1, 0 })) *
            this->scale;
}