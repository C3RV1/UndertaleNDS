//
// Created by cervi on 10/09/2022.
//

#ifndef UNDERTALE_FLOWEY_ATTACK_HPP
#define UNDERTALE_FLOWEY_ATTACK_HPP

#include "Battle/BattleAttack.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"

namespace BtlAttacks {
    class FloweyAttack : public BattleAttack {
    public:
        FloweyAttack();

        bool update() override;

    private:
        static constexpr int kFirstStageFrames = 120;
        static constexpr int kPelletX = 30, kPelletY = -30, kPelletSpacing = 40, kPelletMoveY = 60;
        static constexpr int kPelletSpeedY = (60 << 8) / 60;
        static constexpr int kPelletRadius = 12;
        int _counter = 0, _stage = 0;
        Engine::Texture _pelletTex;
        Engine::Sprite _pelletSpr[5]; // 5 pellets
        int _pelletVecX[5] = {0};
    };
}

#endif //UNDERTALE_FLOWEY_ATTACK_HPP
