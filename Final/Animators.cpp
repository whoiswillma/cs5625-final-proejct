//
// Created by William Ma on 5/12/22.
//

#include "Animators.h"

Animators::Animators(
        const std::shared_ptr<Scene> &scene,
        const std::shared_ptr<OceanScene> &oceanScene
) : birdAnimator(scene),
    oceanAnimator(oceanScene) {
    addAnimators(scene->root);
}

void Animators::addAnimators(const std::shared_ptr<Node> &node) {
    if (BoatNodeAnimator::is_boat(node->name)) {
        boatAnimators.emplace_back(node);
        std::cout << "Found boat node " << node->name << std::endl;
    } else if (SunLightNodeAnimator::is_sun_light(node->name)) {
        sunLightAnimators.emplace_back(node);
        std::cout << "Found sun light node " << node->name << std::endl;
    }

    for (auto &child: node->children) {
        addAnimators(child);
    }
}
