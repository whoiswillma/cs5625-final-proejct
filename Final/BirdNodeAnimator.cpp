//
// Created by Adith Ramachandran on 5/13/22.
//

#include "BirdNodeAnimator.h"
#include <glm/gtx/transform.hpp>

bool BirdNodeAnimator::scatter = false;
double BirdNodeAnimator::prevT = 0.0;

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
    double deltaT = time - BirdNodeAnimator::prevT;
    BirdNodeAnimator::prevT = time;
    /* returns the vector we need to add to the position of the current boid to move it
     * 1% of the way to the center of mass of its neighbors
     */
    const auto center_of_mass = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgPosition(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgPosition += this->birds[currNeighbor].position;
        }
        avgPosition /= glm::max((unsigned long)(this->birds.size() - 1), (unsigned long)1);
        return (avgPosition - this->birds[currBoid].position) / 150.f;
    };

    /* returns the vector we need to add to the position of the current boid to prevent collision with
     * other boids
     */
    const auto course_correction = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 correctionAmt(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) {
                float distance = glm::distance(this->birds[currBoid].position, this->birds[currNeighbor].position);
                const float dist = 5.0f;
                if (distance <= dist) {
                    correctionAmt -=
                            (this->birds[currNeighbor].position - this->birds[currBoid].position) *
                            ((dist - distance) / dist) / 1.5f;
                }
            }
        }

        // if a bird is going to go out of bounds, make it turn hard
        for (auto & wall : Bird::walls) {
            float distance_to_wall = glm::dot(wall.normal, this->birds[currBoid].position - wall.point);
            if (distance_to_wall <= 8.0) {
                // Apply a force in the direction of the wall's normal.
                float correction_magnitude = glm::clamp(exp(-distance_to_wall - 20.0f), 0.0f, 0.2f);

                glm::vec4 dir4;
                switch (currBoid % 3) {
                    case 0:
                        dir4 = glm::rotate(
                            glm::pi<float>() / 4,
                            glm::vec3(0, 1, 0)
                        ) * glm::vec4(wall.normal, 0);
                        break;
                    case 1:
                        dir4 = glm::rotate(
                            0.0f,
                            glm::vec3(0, 1, 0)
                        ) * glm::vec4(wall.normal, 0);
                        break;
                    default:
                        dir4 = glm::rotate(
                            -glm::pi<float>() / 6,
                            glm::vec3(0, 1, 0)
                        ) * glm::vec4(wall.normal, 0);
                        break;
                }
                glm::vec3 dir = glm::normalize(glm::vec3(dir4.x, dir4.y, dir4.z));
                correctionAmt += correction_magnitude * dir;
            }
        }
        return correctionAmt;
    };

    /* returns the vector that we need to add to the velocity of the current boid to get the new velocity */
    const auto center_of_velocity = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgVelocity(0);
        for (size_t currNeighbor = 0; currNeighbor < this->birds.size(); currNeighbor++) {
            if (currBoid != currNeighbor) avgVelocity += this->birds[currNeighbor].velocity;
        }
        avgVelocity /= glm::max((unsigned long)(this->birds.size() - 1), (unsigned long)1);
        return (avgVelocity - this->birds[currBoid].velocity) / 160.f;
    };

    const auto calc_scatter = [&](const size_t currBoid) -> glm::vec3 {
        glm::vec3 avgPosition(0);
        for (const auto& bird : this->birds) {
            avgPosition += bird.position;
        }
        avgPosition /= glm::max((unsigned long)(this->birds.size()), (unsigned long)1);

        glm::vec3 scatterVector =  this->birds[currBoid].position - avgPosition;
        const auto scatterFactor = 1.f / glm::length(scatterVector);
        return glm::normalize(scatterVector) * scatterFactor * 250.f;
    };

    glm::vec3 currWind = Bird::wind.get_wind_dir(time);
    for (size_t currBoid = 0; currBoid < this->birds.size(); currBoid++) {
        glm::vec3 deltaV = center_of_mass(currBoid) +
                           course_correction(currBoid)  +
                           center_of_velocity(currBoid);
        if (BirdNodeAnimator::scatter) {
            deltaV += calc_scatter(currBoid);
        }
        deltaV += currWind / 200.f;
        deltaV *= 51.14f;
        deltaV.y = 0;
        this->birds[currBoid].velocity += deltaV * (float) deltaT;
        this->birds[currBoid].position += this->birds[currBoid].velocity * (float) deltaT;
        this->birds[currBoid].update_self(deltaV);
    }
    if (BirdNodeAnimator::scatter) BirdNodeAnimator::scatter = false;
}