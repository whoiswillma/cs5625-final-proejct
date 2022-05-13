//
// Created by William Ma on 5/12/22.
//

#ifndef CS5625_ANIMATORS_H
#define CS5625_ANIMATORS_H

#include "Scene.h"
#include "BoatNodeAnimator.h"
#include "BirdNodeAnimator.h"
#include "OceanAnimator.h"
#include "OceanScene.h"
#include "SunLightNodeAnimator.h"
#include <memory>

// TODO: refactor lbs animation into a LbsNodeAnimator, and add it here
class Animators {
    void addAnimators(const std::shared_ptr<Node>& node);

public:
    std::vector<BoatNodeAnimator> boatAnimators;
    BirdNodeAnimator birdAnimator;
    OceanAnimator oceanAnimator;
    std::vector<SunLightNodeAnimator> sunLightAnimators;

    explicit Animators(const std::shared_ptr<Scene>& scene, const std::shared_ptr<OceanScene>& oceanScene);
};


#endif //CS5625_ANIMATORS_H
