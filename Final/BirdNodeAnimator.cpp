//
// Created by Adith Ramachandran on 5/13/22.
//

#include "BirdNodeAnimator.h"

bool BirdNodeAnimator::scatter = false;

BirdNodeAnimator::BirdNodeAnimator(std::shared_ptr<Scene> scene) {
    add_birds(scene->root);
}

void BirdNodeAnimator::add_birds(std::shared_ptr<Node> currNode) {
    if (Bird::is_bird(currNode->name)) {
        this->birds.emplace_back(currNode);
    }
    for (std::shared_ptr<Node> child : currNode->children) {
        add_birds(child);
    }
}

void BirdNodeAnimator::speed_up_birds() {
    for (auto& bird : this->birds) {
        bird.velocity += (glm::vec3 { 0.2, 0, 0.2 }) * bird.velocity;
    }
}

void BirdNodeAnimator::slow_down_birds() {
    for (auto& bird : this->birds) {
        bird.velocity -= (glm::vec3 { 0.2, 0, 0.2 }) * bird.velocity;
    }
}

void BirdNodeAnimator::animate_birds(double time) {
    /* returns the vector we need to add to the position of the current boid to move it
     * 1% of the way to the center of mass of its neighbors
     */
    const auto center_of_mass = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgPosition(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgPosition += this->birds[currNeighbor].position;
        }
        avgPosition /= glm::max((unsigned long)(this->birds.size() - 1), (unsigned long)1);
        return (avgPosition - this->birds[currBoid].position) / 5000.f;
    };

    /* returns the vector we need to add to the position of the current boid to prevent collision with
     * other boids
     */
    const auto course_correction = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 correctionAmt(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) {
                float distance = glm::distance(this->birds[currBoid].position, this->birds[currNeighbor].position);
                if (distance <= 0.25f) {
                    correctionAmt -=
                            (this->birds[currNeighbor].position - this->birds[currBoid].position);
                }
            }
        }

        // if a bird is going to go out of bounds, make it turn hard
        if (glm::dot(Bird::walls[0].normal, this->birds[currBoid].position - Bird::walls[0].point) <= 1.0)
            correctionAmt.x -= 0.002;
        if (glm::dot(Bird::walls[1].normal, this->birds[currBoid].position - Bird::walls[1].point) <= 1.0)
            correctionAmt.x += 0.002;
        if (glm::dot(Bird::walls[2].normal, this->birds[currBoid].position - Bird::walls[2].point) <= 1.0)
            correctionAmt.z -= 0.002;
        if (glm::dot(Bird::walls[3].normal, this->birds[currBoid].position - Bird::walls[3].point) <= 1.0)
            correctionAmt.z += 0.002;

        return correctionAmt;
    };

    /* returns the vector that we need to add to the velocity of the current boid to get the new velocity */
    const auto center_of_velocity = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgVelocity(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgVelocity += this->birds[currNeighbor].velocity;
        }
        avgVelocity /= glm::max((unsigned long)(this->birds.size() - 1), (unsigned long)1);
        return (avgVelocity - this->birds[currBoid].velocity) / 800.f;
    };

    const auto calc_scatter = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgPosition(0);
        for (const auto& bird : this->birds) {
            avgPosition += bird.position;
        }
        avgPosition /= glm::max((unsigned long)(this->birds.size()), (unsigned long)1);

        glm::vec3 scatterVector =  this->birds[currBoid].position - avgPosition;
        const auto scatterFactor = 1.f / glm::length(scatterVector);
        return glm::normalize(scatterVector) * scatterFactor;
    };

    glm::vec3 currWind = Bird::wind.get_wind_dir(time);
    for (size_t currBoid = 0; currBoid < this->birds.size(); currBoid++) {
        glm::vec3 deltaV = center_of_mass(currBoid) +
                           course_correction(currBoid)  +
                           center_of_velocity(currBoid);
        if (BirdNodeAnimator::scatter) {
            deltaV += calc_scatter(currBoid);
        }
        deltaV += currWind / 100.f;
        deltaV.y = 0;
        this->birds[currBoid].velocity += deltaV;
        this->birds[currBoid].position += this->birds[currBoid].velocity;
        this->birds[currBoid].update_self(deltaV);
    }
    if (BirdNodeAnimator::scatter) BirdNodeAnimator::scatter = false;
}