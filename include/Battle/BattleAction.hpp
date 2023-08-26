//
// Created by cervi on 02/10/2022.
//

#ifndef UNDERTALE_BATTLE_ACTION_HPP
#define UNDERTALE_BATTLE_ACTION_HPP

#include "Engine/Font.hpp"
#include "Battle/Enemy.hpp"
#include <array>

enum BattleActionState {
    PRINTING_FLAVOR_TEXT,
    CHOOSING_ACTION,
    CHOOSING_TARGET,
    CHOOSING_ACT,
    CHOOSING_ITEM,
    CHOOSING_MERCY,
    MOVING_BUTTON_IN,
    MOVING_BUTTON_OUT,
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
    const s32 _buttonPositions[4][2]{
        {12 << 8, 96 << 8},
        {134 << 8, 96 << 8},
        {12 << 8, 144 << 8},
        {134 << 8, 144 << 8}
    };

public:
    explicit BattleAction(std::vector<Enemy>* enemies, int flavorTextId);
    bool update();
    int getActionNum() const;
    ~BattleAction();
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
    bool updateMovingButtonIn();
    bool updateMovingButtonOut();
    bool updateFighting();

    Engine::Font _fnt;

    std::vector<Enemy>* _enemies;

    std::array<Engine::Texture, 4> _textures;
    std::array<Engine::Sprite, 4> _btn;
    Engine::Texture _smallHeartTex, _bigHeartTex;
    Engine::Sprite _bigHeartSpr, _smallHeartSpr;

    std::string _flavorText;
    Engine::Texture _flavorTextTex;
    Engine::Sprite _flavorTextSpr;

    Engine::Background _fightBoard;
    Engine::Texture _attackTex, _damageNumbers, _missText;
    Engine::Sprite _attackSpr;

    int _gfxAnimId, _activeAnimId;
    BattleActionState _cState = CHOOSING_ACTION;

    int _cAction = 0;
    u8 _cTarget = 0;
    u8 _cPage = 0;
    u8 _cAct = 0;

    constexpr static int _moveFrames = 15;
    constexpr static int _topLeftSpacing = 10 << 8;
    int _movingFrameCount = 0;

    bool _mercyFlee = false;
    std::string _mercyText;
};

#endif //UNDERTALE_BATTLE_ACTION_HPP
