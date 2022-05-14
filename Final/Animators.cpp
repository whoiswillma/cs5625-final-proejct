//
// Created by William Ma on 5/12/22.
//

#include "Animators.h"

Animators::Animators(
        const std::shared_ptr<Scene> &scene,
        const std::shared_ptr<OceanScene> &oceanScene
) : birdAnimator(scene),
    oceanAnimator(oceanScene) {
    addAnimators(scene, scene->root);

    for (const auto& light : scene->pointLights) {
        if (light->name.find("SunLight") != std::string::npos) {
            auto node = scene->findNode("SunLightNode");
            sunLightAnimators.emplace_back(light, node);
            std::cout << "Found Sun Light" << std::endl;
        }
    }
}

void Animators::addAnimators(const std::shared_ptr<Scene>& scene, const std::shared_ptr<Node> &node) {
    if (BoatNodeAnimator::is_boat(node->name)) {
        boatAnimators.emplace_back(node);
        std::cout << "Found boat node " << node->name << std::endl;
    }

    for (auto &child: node->children) {
        addAnimators(scene, child);
    }
}
