//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_BATTLE_HPP
#define UNDERTALE_BATTLE_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Engine/Engine.hpp"
#include "ManagedSprite.hpp"
#include "Cutscene/Navigation.hpp"
#include "BattleAttack.hpp"

struct Enemy {
    uint16_t enemyId = 0;
    char enemyName[20] = {0};
    uint16_t hp = 0;
    uint16_t maxHp = 0;
};

// TODO: Display health

class Battle {
public:
    Battle();
    void loadFromStream(FILE* stream);
    void draw() const;
    void update();
    void free_();
    void resetBattleAttack();
    bool running = true;
    Navigation nav;

    uint8_t enemyCount = 0;
    Enemy* enemies = nullptr;

    uint8_t textureCount = 0;
    Engine::Texture** textures = nullptr;

    uint8_t spriteCount = 0;
    ManagedSprite** sprites = nullptr;

    Engine::Background bulletBoard;
    uint8_t boardX = 0, boardY = 0, boardW = 0, boardH = 0;

    const int32_t playerSpeed = (60 << 8) / 60;
    Engine::Texture player;
    Engine::Sprite playerManager;

    BattleAttack* currentBattleAttack = nullptr;
    bool hitFlag = false;
};

void runBattle(FILE* stream);
extern Battle* globalBattle;

#endif //UNDERTALE_BATTLE_HPP
