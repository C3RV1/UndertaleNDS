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
    constexpr static s32 kAttackSpeed = (5 << 8); // 5 pixels per frame

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

    bool _freed = false;

    Engine::Font _fnt;

    u8 _enemyCount = 0;
    Enemy* _enemies = nullptr;

    Engine::Texture _fightTex, _actTex, _itemTex, _mercyTex, _bigHeartTex, _smallHeartTex;
    Engine::Sprite _fightBtn, _actBtn, _itemBtn, _mercyBtn, _heartSpr;

    Engine::Background _fightBoard;
    Engine::Texture _attackTex, _damageNumbers, _missText;
    Engine::Sprite _attackSpr;

    int _gfxAnimId, _activeAnimId;
    BattleActionState _cState = CHOOSING_ACTION;

    int _cAction = 0;
    u8 _cTarget = 0;
    u8 _cPage = 0;
    u8 _cAct = 0;

    bool _mercyFlee = false;
    char* _mercyText = nullptr;
};

#endif //UNDERTALE_BATTLE_ACTION_HPP
