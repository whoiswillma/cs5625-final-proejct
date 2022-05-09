//
// Created by Adith Ramachandran on 5/8/22.
//

#include "Bird.hpp"

Bird::Bird(std::shared_ptr<Node> birdNode) {
    std::random_device rd;
    std::mt19937_64 mt(rd());
    std::uniform_int_distribution<int> dist_int(-10, 10);
    std::uniform_real_distribution<float> dist_real(-1.0, 1.0);

    this->nodePtr = birdNode;
    this->position = glm::vec3{ dist_int(mt), 7.5, dist_int(mt) };
    this->orientation = glm::normalize(glm::qua<float>{ dist_real(mt), dist_real(mt), dist_real(mt), dist_real(mt) });

    this->nodePtr->transform =
            glm::translate(glm::mat4(1), this->position) *
            glm::mat4_cast(this->orientation) * this->nodePtr->transform;
}
