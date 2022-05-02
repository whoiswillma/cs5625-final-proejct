#include "Scene.h"

glm::mat4 Node::getTransformTo(const std::shared_ptr<Node>& other) {
    if (other == nullptr) {
        auto acc = glm::identity<glm::mat4>();

        for (Node* node = this; node != nullptr; node = node->parent.get()) {
            acc = node->transform * acc;
        }

        return acc;

    } else {
        return glm::inverse(other->getTransformTo(nullptr)) * getTransformTo(nullptr);
    }
}
