//
// Created by Adith Ramachandran on 5/13/22.
//

#pragma once

#include <vector>
#include <memory>
#include "Bird.hpp"
#include "Scene.h"

class BirdNodeAnimator
{
public:
    BirdNodeAnimator(std::shared_ptr<Scene> scene);
    void speed_up_birds();
    void slow_down_birds();
    void animate_birds(double time);
    static bool scatter;
    static double prevT;

private:
    std::vector<Bird> birds;
    void add_birds(std::shared_ptr<Node> currNode);
};
