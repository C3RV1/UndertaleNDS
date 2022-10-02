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
    CHOOSING_MERCY
};

enum BattleActions {
    ACTION_FIGHT = 0,
    ACTION_ACT = 1,
    ACTION_ITEM = 2,
    ACTION_MERCY = 3
};

class BattleAction {
public:
    BattleAction(u8 enemyCount, Enemy* enemies);
    void enter(BattleActionState state);
    void setBtn();
    bool update();
    bool updateChoosingAction();
    bool updateChoosingTarget();
    bool updateChoosingAct();
    bool updateChoosingMercy();
private:
    Engine::Font fnt;
    u8 enemyCount = 0;
    Enemy* enemies = nullptr;
    Engine::Texture fightTex, actTex, itemTex, mercyTex, heartTex;
    Engine::Sprite fightBtn, actBtn, itemBtn, mercyBtn, heartSpr;
    int gfxAnimId, activeAnimId;
    BattleActionState currentState = CHOOSING_ACTION;
    int currentAction = 0;
    u8 chosenTarget = 0;
};

#endif //UNDERTALE_BATTLE_ACTION_HPP
