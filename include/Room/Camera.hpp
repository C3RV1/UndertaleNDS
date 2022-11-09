//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_CAMERA_HPP
#define UNDERTALE_CAMERA_HPP

#include "Engine/Sprite.hpp"

class Camera {
public:
    Camera() : _pos(Engine::NoAlloc) {}
    void updatePosition(bool roomChange);
    bool _manual = false;
    int _prevX = 0, _prevY = 0;
    // top left camera position
    Engine::Sprite _pos;
};

extern Camera globalCamera;

#endif //UNDERTALE_CAMERA_HPP
