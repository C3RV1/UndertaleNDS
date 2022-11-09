//
// Created by cervi on 10/09/2022.
//

#ifndef UNDERTALE_FLOWEY_ATTACK2_HPP
#define UNDERTALE_FLOWEY_ATTACK2_HPP

#include "Battle/BattleAttack.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"

namespace BtlAttacks {
    class FloweyAttack2 : public BattleAttack {
    public:
        FloweyAttack2();
        ~FloweyAttack2() noexcept override;
        bool update() override;
    private:
        static const int kShowFrames = 4;
        int _counter = 0;
        int _stage = 0;
        int _cPelletShown = 0;
        static const int kPelletW = 14;
        static const int kPelletX = 55, kPelletY = 50, kPelletSpacingX = 10, kPelletSpacingY = 90;
        static const int kPelletSpeed = (10 << 8) / 60;
        static const int kPelletRadius = 15;
        Engine::Texture _pelletTex;
        Engine::Sprite* _pelletSpr[kPelletW * 2] = {nullptr};
    };
}

#endif //UNDERTALE_FLOWEY_ATTACK2_HPP
