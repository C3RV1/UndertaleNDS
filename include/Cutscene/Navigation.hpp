//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_NAVIGATION_HPP
#define LAYTON_NAVIGATION_HPP

class Navigation;

#include "SpriteManager.hpp"

enum NavigationTaskType {
    POSITION = 0,
    SCALE = 1
};

enum TargetType {
    NULL_ = 0,
    PLAYER = 1,
    SPRITE = 2,
    CAMERA = 3
};

struct NavigationTask {
    uint16_t startingX = 0, startingY = 0;
    uint16_t destX = 0, destY = 0;
    uint16_t frames = 0;
    uint16_t currentFrames = 0;
    NavigationTaskType taskType = POSITION;
    Engine::SpriteManager* target = nullptr;
};

class Navigation {
public:
    void startTask(NavigationTask* navTask);
    void update();
private:
    uint8_t moveTaskCount = 0;
    NavigationTask** moveTasks = nullptr;
};

#include "Room.hpp"

#endif //LAYTON_NAVIGATION_HPP
