//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttacks/MovementTutorial.hpp"

namespace BtlAttacks {
    MovementTutorial::MovementTutorial() : tutorialSpr(Engine::Allocated3D) {
        FILE *f = fopen("nitro:/spr/cutscene/0/spr_guidearrows.cspr", "rb");
        if (f) {
            int loadTex = tutorialTex.loadCSPR(f);
            if (loadTex != 0) {
                char buffer[100];
                sprintf(buffer, "Error loading guide arrows: %d", loadTex);
                nocashMessage(buffer);
            }
        } else {
            nocashMessage("Error opening guide arrows");
        }
        fclose(f);

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
