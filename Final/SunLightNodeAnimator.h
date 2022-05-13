//
// Created by William Ma on 5/13/22.
//

#ifndef CS5625_SUNLIGHTNODEANIMATOR_H
#define CS5625_SUNLIGHTNODEANIMATOR_H

#include <memory>
#include "Scene.h"

class SunLightNodeAnimator {
    std::shared_ptr<Node> node;

public:
    static bool is_sun_light(const std::string& nodeName) {
        return nodeName == "SunLight";
    };

    explicit SunLightNodeAnimator(std::shared_ptr<Node>  sunLightNode);
    void update(float thetaSun);
};


#endif //CS5625_SUNLIGHTNODEANIMATOR_H
