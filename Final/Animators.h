//
// Created by William Ma on 5/12/22.
//

#ifndef CS5625_ANIMATORS_H
#define CS5625_ANIMATORS_H

#include "Scene.h"
#include "BoatNodeAnimator.h"
#include <memory>

// TODO: refactor lbs animation into a LbsNodeAnimator, and add it here
// TODO: refactor bird animation into a BirdNodeAnimator, and add it here
// TODO: refactor ocean animation into a OceanNodeAnimator, and add it here
class Animators {
    void addAnimators(const std::shared_ptr<Node>& node);

public:
    std::vector<BoatNodeAnimator> boatAnimators;

    explicit Animators(const std::shared_ptr<Scene>& scene);
};


#endif //CS5625_ANIMATORS_H
