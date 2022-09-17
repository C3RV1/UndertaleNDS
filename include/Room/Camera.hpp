//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_CAMERA_HPP
#define UNDERTALE_CAMERA_HPP

class Camera;

#include "Engine/Engine.hpp"
#include "Room.hpp"
#include "Player.hpp"
#include "math.hpp"

class Camera {
public:
    Camera() : pos(Engine::NoAlloc) {}
    void updatePosition(bool roomChange);
    bool manual = false;
    int prevX = 0, prevY = 0;
    // top left camera position
    Engine::Sprite pos;
};

extern Camera globalCamera;

#endif //UNDERTALE_CAMERA_HPP
