//
// Created by cervi on 10/09/2022.
//

#ifndef LAYTON_FLOWEYATTACK_HPP
#define LAYTON_FLOWEYATTACK_HPP

#include "Battle/BattleAttack.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"

namespace BtlAttacks {
    class FloweyAttack : public BattleAttack {
    public:
        FloweyAttack();
        ~FloweyAttack() noexcept override;
        bool update() override;
    private:
        const int firstStageFrames = 200;
        const int pelletX = 30, pelletY = -30, pelletSpacing = 40, pelletMoveY = 60;
        const int pelletSpeedY = (10 << 8) / 60;
        int counter = 0, stage = 0;
        Engine::Texture pelletTex;
        Engine::Sprite* pellets[5] = {nullptr}; // 5 pellets
        int pelletVecX[5] = {0};
    };
}

#endif //LAYTON_FLOWEYATTACK_HPP
