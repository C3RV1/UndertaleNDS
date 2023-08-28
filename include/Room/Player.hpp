//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_PLAYER_HPP
#define UNDERTALE_PLAYER_HPP

#include "Engine/Sprite.hpp"
#include <cstdio>
#include <nds.h>

class Player {
public:
    constexpr static int kMoveSpeed = (90 << 8) / 60;  // 90 pixels per second, debug speed
    // const int MOVE_SPEED = (70 << 8) / 60;  // 70 pixels per second

    Player();
    void update();
    bool check_collisions() const;
    void check_interact() const;
    void check_exits();
    void setPlayerControl(bool playerControl);
    void draw();

    // sprite top left position
    Engine::Sprite _playerSpr;
private:
    Engine::Texture _playerTex;

    // animation ids
    bool _playerControl = true;
    int _upIdleId;
    int _downIdleId;
    int _leftIdleId;
    int _rightIdleId;
    int _upMoveId;
    int _downMoveId;
    int _leftMoveId;
    int _rightMoveId;
};

extern Player* globalPlayer;

#endif //UNDERTALE_PLAYER_HPP
