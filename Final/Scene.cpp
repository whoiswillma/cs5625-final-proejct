#include "Scene.h"
#include <memory>

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

template <class T>
T interpNoWrap(double tick, std::map<double, T> keyframes, std::function<T(T, T, double)> mix) {
    auto upper = keyframes.upper_bound(tick);
    auto lower = upper; // assign lower to have the same type as upper
    double alpha;

    if (upper == keyframes.end()) {
        return (--upper)->second;
    } else if (upper == keyframes.begin()) {
        return upper->second;
    } else {
        lower = --keyframes.upper_bound(tick);
        alpha = (tick - lower->first) / (upper->first - lower->first);
        return mix(lower->second, upper->second, (float) alpha);
    }
}

double fmodulus(double x, double y) {
    return x - y * floor(x / y);
}

void Scene::animate(double time, unsigned int animationIdx) {
    if (animationIdx >= animations.size()) {
        return;
    }
    Animation animation = animations[animationIdx];
    double tick = fmodulus(animation.ticksPerSecond * time, animation.duration);

    for (const auto& channel : animation.channels) {
        std::function<glm::vec3(glm::vec3, glm::vec3, double)> mix = [](glm::vec3 lhs, glm::vec3 rhs, double alpha) {
            return glm::mix(lhs, rhs, alpha);
        };

        std::function<glm::quat(glm::quat, glm::quat, double)> slerp = [](glm::quat lhs, glm::quat rhs, double alpha) {
            return glm::slerp(lhs, rhs, (float) alpha);
        };

        auto position = interpNoWrap(tick, channel.translation,  mix);
        auto rotation = interpNoWrap(tick, channel.rotation, slerp);
        auto scale = interpNoWrap(tick, channel.scale, mix);

        std::shared_ptr<Node> node = nameToNode[channel.nodeName];
        const auto I = glm::identity<glm::mat4>();
        node->transform =
                glm::translate(I, position)
                * glm::mat4_cast(rotation)
                * glm::scale(I, scale);
    }
}
