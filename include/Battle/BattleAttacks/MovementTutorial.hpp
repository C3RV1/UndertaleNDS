//
// Created by cervi on 08/09/2022.
//

#ifndef UNDERTALE_MOVEMENT_TUTORIAL_HPP
#define UNDERTALE_MOVEMENT_TUTORIAL_HPP

#include "Battle/BattleAttack.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"

namespace BtlAttacks {
    class MovementTutorial : public BattleAttack {
    public:
        MovementTutorial();
        ~MovementTutorial() noexcept override;
        bool update() override;
    private:
        Engine::Texture tutorialTex;
        Engine::Sprite tutorialSpr;
    };
}

#endif //UNDERTALE_MOVEMENT_TUTORIAL_HPP
