//
// Created by cervi on 10/09/2022.
//

#ifndef LAYTON_FLOWEYATTACK2_HPP
#define LAYTON_FLOWEYATTACK2_HPP

#include "Battle/BattleAttack.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/math.hpp"

namespace BtlAttacks {
    class FloweyAttack2 : public BattleAttack {
    public:
        FloweyAttack2();
        ~FloweyAttack2() noexcept override;
        bool update() override;
    private:
        static const int showFrames = 4;
        int counter = 0;
        int stage = 0;
        int currentPelletShown = 0;
        static const int pelletW = 14;
        static const int pelletX = 55, pelletY = 50, pelletSpacingX = 10, pelletSpacingY = 90;
        static const int pelletSpeed = (20 << 8) / 60;
        static const int pelletRadius = 15;
        Engine::Texture pelletTex;
        Engine::Sprite* pellets[pelletW * 2] = {nullptr};
    };
}

#endif //LAYTON_FLOWEYATTACK2_HPP
