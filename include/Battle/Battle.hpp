//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_BATTLE_HPP
#define LAYTON_BATTLE_HPP

class Battle;
extern Battle* globalBattle;

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Room/Room.hpp"
#include "Background.hpp"
#include "Texture.hpp"
#include "Sprite.hpp"
#include "ManagedSprite.hpp"
#include "Cutscene/Navigation.hpp"
#include "BattleAttack.hpp"
#include "Engine.hpp"

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
    void draw();
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

#endif //LAYTON_BATTLE_HPP
