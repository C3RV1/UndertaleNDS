//
// Created by cervi on 10/09/2022.
//

#ifndef UNDERTALE_FLOWEYATTACK_HPP
#define UNDERTALE_FLOWEYATTACK_HPP

#include "Battle/BattleAttack.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/math.hpp"

namespace BtlAttacks {
    class FloweyAttack : public BattleAttack {
    public:
        FloweyAttack();
        ~FloweyAttack() noexcept override;
        bool update() override;
    private:
        static const int firstStageFrames = 120;
        static const int pelletX = 30, pelletY = -30, pelletSpacing = 40, pelletMoveY = 60;
        static const int pelletSpeedY = (60 << 8) / 60;
        static const int pelletRadius = 12;
        int counter = 0, stage = 0;
        Engine::Texture pelletTex;
        Engine::Sprite* pellets[5] = {nullptr}; // 5 pellets
        int pelletVecX[5] = {0};
    };
}

#endif //UNDERTALE_FLOWEYATTACK_HPP
