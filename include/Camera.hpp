//
// Created by cervi on 27/08/2022.
//

#ifndef LAYTON_CAMERA_HPP
#define LAYTON_CAMERA_HPP

class Camera;

#include "Room.hpp"
#include "Player.hpp"
#include "math.hpp"

class Camera {
public:
    void updatePosition(bool roomChange);
    // top left camera position
    int32_t x = 0, y = 0;  // 23 bit integer part, 8 bit fractional part
};

extern Camera globalCamera;

#endif //LAYTON_CAMERA_HPP
