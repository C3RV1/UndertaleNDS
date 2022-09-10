//
// Created by cervi on 10/09/2022.
//
#include "Battle/BattleAttacks/FloweyAttack2.hpp"

//
// Created by cervi on 10/09/2022.
//

#include "Battle/BattleAttacks/FloweyAttack.hpp"

namespace BtlAttacks {
    FloweyAttack2::FloweyAttack2() {
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

        int i = 0;
        for (auto & pellet : pellets) {
            pellet = new Engine::Sprite(Engine::Allocated3D);
            pellet->loadTexture(pelletTex);
            // Set pellets in clockwise order
            // Two rows, one at top and one at bottom
            // Of pellets
            if (i < pelletW) {
                pellet->wx = (pelletX + i * pelletSpacingX) << 8;
                pellet->wy = pelletY << 8;
            } else if (i - pelletW < pelletW) {
                int i2 = i - pelletW;
                pellet->wx = ((pelletX + (pelletW - 1 - i2) * pelletSpacingX) << 8);
                pellet->wy = (pelletY + pelletSpacingY) << 8;
            }
            pellet->setShown(false);
            i++;
        }
    }

    bool FloweyAttack2::update() {
        if (stage == 0) {
            counter++;
            if (counter < showFrames)
                return false;
            counter = 0;
            pellets[currentPelletShown]->setShown(true);
            currentPelletShown++;
            if (currentPelletShown == pelletW * 2) {
                stage = 1;
            }
        } else {
            for (int i = 0; i < pelletW * 2; i++) {
                auto pellet = pellets[i];
                if (i < pelletW) {
                    pellet->wy += pelletSpeed;
                } else if (i - pelletW < pelletW) {
                    pellet->wy -= pelletSpeed;
                }
                if (distSquared_fp(pellet->wx + (4 << 8),
                                   pellet->wy + (4 << 8),
                                   globalBattle->playerManager.wx + (9 << 8) / 2,
                                   globalBattle->playerManager.wy + (9 << 8) / 2) <= (pelletRadius * pelletRadius) << 8) {
                    saveGlobal.hp = 20;
                    return true;
                }
            }
        }
        return false;
    }

    FloweyAttack2::~FloweyAttack2() noexcept {
        for (auto& pellet : pellets) {
            pellet->setShown(false);
            delete pellet;
        }

        pelletTex.free_();
    }
}