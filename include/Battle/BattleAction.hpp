//
// Created by cervi on 02/10/2022.
//

#ifndef UNDERTALE_BATTLE_ACTION_HPP
#define UNDERTALE_BATTLE_ACTION_HPP

#include "Engine/Font.hpp"
#include "Battle/Enemy.hpp"

enum BattleActionState {
    CHOOSING_ACTION,
    CHOOSING_TARGET,
    CHOOSING_ACT,
    CHOOSING_ITEM,
    CHOOSING_MERCY,
    FIGHTING
};

enum BattleActions {
    ACTION_FIGHT = 0,
    ACTION_ACT = 1,
    ACTION_ITEM = 2,
    ACTION_MERCY = 3
};

class BattleAction {
    const s32 attackSpeed = (5 << 8); // 5 pixels per frame

public:
    BattleAction(u8 enemyCount, Enemy* enemies);
    bool update();
    int getActionNum() const;
    void free_();
    ~BattleAction() {free_();}
private:
    void setBtn();
    void enter(BattleActionState state);

    void drawAct(bool draw);
    void drawMercy(bool draw);
    void drawTarget();

    bool updateChoosingAction();
    bool updateChoosingTarget();
    bool updateChoosingAct();
    bool updateChoosingMercy();
    bool updateChoosingItem();
    bool updateFighting();

    bool freed = false;

    Engine::Font fnt;

    u8 enemyCount = 0;
    Enemy* enemies = nullptr;

    Engine::Texture fightTex, actTex, itemTex, mercyTex, bigHeartTex, smallHeartTex;
    Engine::Sprite fightBtn, actBtn, itemBtn, mercyBtn, heartSpr;

    Engine::Background fightBoard;
    Engine::Texture attackTexture;
    Engine::Sprite attackSprite;

    int gfxAnimId, activeAnimId;
    BattleActionState currentState = CHOOSING_ACTION;

    int currentAction = 0;
    u8 chosenTarget = 0;
    u8 currentPage = 0;
    u8 chosenAct = 0;

    bool mercyFlee = false;
    char* mercyText = nullptr;
};

#endif //UNDERTALE_BATTLE_ACTION_HPP
