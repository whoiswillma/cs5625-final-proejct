//
// Created by Adith Ramachandran on 5/8/22.
//

#include "Bird.hpp"
#include <glm/gtx/io.hpp>

const double bound = 10.0;
const float randomVelocity = 0.25;
std::vector<Wall> Bird::walls {
        Wall{ glm::vec3 {  bound, bound,  0 }, glm::normalize(glm::vec3{  -bound, 0,  0 }) },
        Wall{ glm::vec3 { -bound, bound,  0 }, glm::normalize(glm::vec3{   bound, 0,  0 }) },
        Wall{ glm::vec3 {  0,   bound,  bound }, glm::normalize(glm::vec3{ 0,   0, -bound }) },
        Wall{ glm::vec3 {  0,   bound, -bound }, glm::normalize(glm::vec3{ 0,   0,  bound }) }
};

Bird::Bird(std::shared_ptr<Node> birdNode) {
    std::random_device rd;
    std::mt19937_64 mt(rd());
    std::uniform_int_distribution<int> dist_int(-(int) bound, (int) bound);
    std::uniform_real_distribution<float> dist_real(-randomVelocity, randomVelocity);

    this->nodePtr = birdNode;
    this->position = glm::vec3{ dist_int(mt), 7.5, dist_int(mt) };
    this->velocity = glm::vec3{ dist_real(mt), 0, dist_real(mt) };
    this->initVelocity = glm::length(this->velocity);
    glm::mat4 scaleMat = glm::scale(glm::mat4(1), glm::vec3{
        glm::length(glm::vec3(this->nodePtr->transform[0])),
        glm::length(glm::vec3(this->nodePtr->transform[1])),
        glm::length(glm::vec3(this->nodePtr->transform[2]))
    });
    glm::mat4 transformCopy(this->nodePtr->transform);
    transformCopy[3][0] = transformCopy[3][1] = transformCopy[3][2] = 0;
    transformCopy[0] /= glm::length(transformCopy[0]);
    transformCopy[1] /= glm::length(transformCopy[1]);
    transformCopy[2] /= glm::length(transformCopy[2]);
    glm::mat4 rotationMat= transformCopy;
    this->rotScale = rotationMat * scaleMat;

    update_self(glm::vec3(0));
}

void Bird::update_self(glm::vec3 deltaV) {

    if (glm::length(this->velocity) > this->initVelocity) this->velocity *= 0.99f;
    glm::qua<float> lookAtQuat = glm::quatLookAt(
            -glm::normalize(glm::vec3{
                this->velocity.x,
                this->velocity.y,
                this->velocity.z
                }),
            glm::vec3{ 0, 1, 0 }
        );
    this->nodePtr->transform =
            glm::translate(glm::mat4(1), this->position) *
            glm::mat4_cast(lookAtQuat) *
            glm::rotate(glm::mat4(1),
                        (deltaV.x + deltaV.z) * 100.f,
                        glm::normalize(this->velocity)) *
            this->rotScale;
}