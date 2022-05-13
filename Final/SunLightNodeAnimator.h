//
// Created by William Ma on 5/13/22.
//

#ifndef CS5625_SUNLIGHTNODEANIMATOR_H
#define CS5625_SUNLIGHTNODEANIMATOR_H

#include <memory>
#include "Scene.h"

class SunLightNodeAnimator {
    std::shared_ptr<PointLight> light;
    std::shared_ptr<Node> node;

public:
    explicit SunLightNodeAnimator(std::shared_ptr<PointLight> light, std::shared_ptr<Node> node);
    void update(float thetaSun, float turbidity);
};


#endif //CS5625_SUNLIGHTNODEANIMATOR_H
