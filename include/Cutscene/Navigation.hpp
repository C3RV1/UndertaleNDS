//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_NAVIGATION_HPP
#define LAYTON_NAVIGATION_HPP

#include "Room.hpp"
#include "SpriteManager.hpp"

struct NavigationMoveTask {
    uint16_t startingX = 0, startingY = 0;
    uint16_t destX = 0, destY = 0;
    uint16_t frames = 0;
    uint16_t currentFrames = 0;
    Engine::SpriteManager* target = nullptr;
};

class Navigation {
public:
    void update();
private:
    uint8_t moveTaskCount = 0;
    NavigationMoveTask* moveTasks = nullptr;
};

#endif //LAYTON_NAVIGATION_HPP
