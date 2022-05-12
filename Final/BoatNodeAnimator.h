//
// Created by William Ma on 5/12/22.
//

#ifndef CS5625_BOATNODEANIMATOR_H
#define CS5625_BOATNODEANIMATOR_H

#include "Scene.h"
#include "Tessendorf.h"
#include <memory>


class BoatNodeAnimator {
    std::shared_ptr<Node> node;

public:
    static bool is_boat(const std::string& nodeName) {
        return nodeName == "paperboat";
    };

    BoatNodeAnimator(std::shared_ptr<Node> boatNode);
    void update(
            tessendorf::array2d<float> displacementMap,
            tessendorf::array2d<float> gradXMap,
            tessendorf::array2d<float> gradZMap,
            glm::mat4 oceanTransform
    );
};


#endif //CS5625_BOATNODEANIMATOR_H
