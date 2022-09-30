//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Battle/Battle.hpp"

namespace BtlAttacks {
    MovementTutorial::MovementTutorial() : tutorialSpr(Engine::Allocated3D) {
        tutorialTex.loadPath("cutscene/0/spr_guidearrows");

        tutorialSpr.loadTexture(tutorialTex);
        tutorialSpr.setShown(true);
        tutorialSpr.wx = globalBattle->playerManager.wx - (10 << 8);
        tutorialSpr.wy = globalBattle->playerManager.wy - (10 << 8);
        tutorialSpr.layer = 50;
        int animId = tutorialSpr.nameToAnimId("tutorial");
        tutorialSpr.setSpriteAnim(animId);
    }

    bool MovementTutorial::update() {
        if (keysDown() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
            return true;
        return false;
    }

    MovementTutorial::~MovementTutorial() noexcept {
        tutorialSpr.setShown(false);
        tutorialTex.free_();
    }
}
