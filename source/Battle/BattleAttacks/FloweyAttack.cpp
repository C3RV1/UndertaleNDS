//
// Created by cervi on 10/09/2022.
//

#include "Battle/BattleAttacks/FloweyAttack.hpp"

namespace BtlAttacks {
    FloweyAttack::FloweyAttack() {
        FILE *f = fopen("nitro:spr/battle/attack_pellets.cspr", "rb");
        if (f) {
            int texLoad = pelletTex.loadCSPR(f);
            if (texLoad != 0) {
                char buffer[100];
                sprintf(buffer, "Error loading attack pellets: %d", texLoad);
                nocashMessage(buffer);
            }
        } else {
            nocashMessage("Error opening attack pellets");
        }

        int x = pelletX;
        for (auto & pellet : pellets) {
            pellet = new Engine::Sprite(Engine::Allocated3D);
            pellet->loadTexture(pelletTex);
            pellet->wx = x << 8;
            pellet->wy = pelletY << 8;
            x += pelletSpacing;
            pellet->setShown(true);
        }
    }

    bool FloweyAttack::update() {
        if (stage == 0) {
            counter++;
            if (counter > firstStageFrames) {
                stage++;
                // TODO: Improve move precision (maybe offset error?)
                int diffY = globalBattle->playerManager.wy + (9 << 8) / 2 - ((pelletY + pelletMoveY) << 8) - (4 << 8);
                int ySteps = (diffY << 8) / pelletSpeedY;
                for (int i = 0; i < 5; i++) {
                    auto pellet = pellets[i];
                    pelletVecX[i] = ((globalBattle->playerManager.wx + (9 << 8) / 2 - pellet->wx - (4 << 8)) << 8) / ySteps;
                }
            } else {
                for (auto & pellet : pellets) {
                    pellet->wy = (pelletY << 8) + ((pelletMoveY * counter) << 8 / firstStageFrames);
                }
            }
        } else {
            for (int i = 0; i < 5; i++) {
                auto pellet = pellets[i];
                pellet->wx += pelletVecX[i];
                pellet->wy += pelletSpeedY;
                if (distSquared_fp(pellet->wx + (4 << 8),
                                   pellet->wy + (4 << 8),
                                   globalBattle->playerManager.wx + (9 << 8) / 2,
                                   globalBattle->playerManager.wy + (9 << 8) / 2) <= (pelletRadius * pelletRadius) << 8) {
                    globalBattle->hitFlag = true;
                    globalSave.hp = 1;
                    return true;
                }
            }
            if (pellets[0]->wy > 180 << 8) {
                globalBattle->hitFlag = false;
                return true;
            }
        }
        return false;
    }

    FloweyAttack::~FloweyAttack() noexcept {
        for (auto& pellet : pellets) {
            pellet->setShown(false);
            delete pellet;
        }

        pelletTex.free_();
    }
}