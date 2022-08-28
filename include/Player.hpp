//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_PLAYER_HPP
#define LAYTON_PLAYER_HPP

class Player;

#include "Sprite.hpp"
#include <stdio.h>
#define ARM9
#include <nds.h>
#include "Sprite3DManager.hpp"
#include "Camera.hpp"
#include "Room.hpp"
#include "math.hpp"

class Player {
public:
    const int MOVE_SPEED = (60 << 8) / 60;  // 128 pixels per second
    const int FRAME_TIME = 15;

    Player();
    void showPlayer();
    void hidePlayer();
    void update(Room*& room, Camera& cam);
    bool check_collisions(Room*& room) const;
    void check_exits(Room*& room, Camera& cam);
    void draw(Camera &cam);

    // sprite top left position
    int32_t x = 0, y = 0;  // 23 bit integer part, 8 bit fractional part
private:
    Engine::Sprite playerSpr;
    Engine::SpriteManager* sprManager = nullptr;
    Engine::SpriteControl* sprControl = nullptr;

    uint8_t currentAnimation = 0;  // 0-3 idle up, down, left, right, 4-7 move same

    // animation ids
    int upIdleId;
    int downIdleId;
    int leftIdleId;
    int rightIdleId;
    int upMoveId;
    int downMoveId;
    int leftMoveId;
    int rightMoveId;
};

#endif //LAYTON_PLAYER_HPP
