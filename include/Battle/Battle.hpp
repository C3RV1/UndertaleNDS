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
#include "BattleAction.hpp"
#include "Enemy.hpp"

// TODO: Display health

class Battle {
public:
    Battle();
    void loadFromStream(FILE* stream);
    void show();
    void hide();
    void update();
    void free_();
    void resetBattleAttack();
    bool shown = false;
    bool running = true;
    Navigation nav;

    u8 enemyCount = 0;
    Enemy* enemies = nullptr;

    u8 textureCount = 0;
    Engine::Texture** textures = nullptr;

    u8 spriteCount = 0;
    ManagedSprite** sprites = nullptr;

    Engine::Background bulletBoard;
    u8 boardX = 0, boardY = 0, boardW = 0, boardH = 0;

    const s32 playerSpeed = (60 << 8) / 60;
    Engine::Texture player;
    Engine::Sprite playerManager;

    BattleAttack* currentBattleAttack = nullptr;
    BattleAction* currentBattleAction = nullptr;
    bool hitFlag = false;
};

void runBattle(FILE* stream);
extern Battle* globalBattle;

#endif //UNDERTALE_BATTLE_HPP
