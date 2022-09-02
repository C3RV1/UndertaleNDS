//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_BATTLE_HPP
#define LAYTON_BATTLE_HPP

class Battle;

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Room.hpp"
#include "ManagedSprite.hpp"
#include "Navigation.hpp"

struct Enemy {
    uint16_t enemyId;
    char enemyName[20];
    uint16_t hp;
    uint16_t maxHp;
};

class Battle {
public:
    void loadFromStream(FILE* stream);
    void draw();
    void update();
    void free_();
    bool running = true;
    Navigation nav;

    uint8_t enemyCount = 0;
    Enemy* enemies = nullptr;

    uint8_t spriteCount = 0;
    ManagedSprite** sprites = nullptr;
};

void runBattle(FILE* stream);
extern Battle* globalBattle;

#endif //LAYTON_BATTLE_HPP
