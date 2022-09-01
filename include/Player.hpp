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
#include "Cutscene/Cutscene.hpp"
#include "InGameMenu.hpp"

class Player {
public:
    const int MOVE_SPEED = (90 << 8) / 60;  // 128 pixels per second
    const int FRAME_TIME = 15;

    Player();
    void update();
    bool check_collisions() const;
    void check_exits();
    void draw();

    // sprite top left position
    Engine::SpriteManager spriteManager;
    bool playerControl = true;
private:
    Engine::Sprite playerSpr;

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

extern Player* globalPlayer;

#endif //LAYTON_PLAYER_HPP
