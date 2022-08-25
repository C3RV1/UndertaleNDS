//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_PLAYER_HPP
#define LAYTON_PLAYER_HPP

#include "Sprite.hpp"

class Player {
public:
    void load();
    void draw();
private:
    Engine::Sprite playerSpr;
    int x, y;
};

Player globalPlayer;

#endif //LAYTON_PLAYER_HPP
