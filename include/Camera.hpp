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
    void updatePosition(Room& room, Player& player, bool roomChange);
    // top left camera position
    int32_t x = 0, y = 0;  // 23 bit integer part, 8 bit fractional part
};

#endif //LAYTON_CAMERA_HPP
