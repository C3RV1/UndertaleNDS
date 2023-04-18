//
// Created by cervi on 10/09/2022.
//

#include "Battle/BattleAttacks/FloweyAttack.hpp"
#include "Battle/Battle.hpp"
#include "Engine/math.hpp"
#include "Save.hpp"

namespace BtlAttacks {
    FloweyAttack::FloweyAttack() {
        _pelletTex.loadPath("battle/attack_pellets");

        int x = kPelletX;
        for (auto & pellet : _pelletSpr) {
            pellet.setAllocationMode(Engine::Allocated3D);
            pellet.loadTexture(_pelletTex);
            pellet._wx = x << 8;
            pellet._wy = kPelletY << 8;
            x += kPelletSpacing;
            pellet.setShown(true);
        }
    }

    bool FloweyAttack::update() {
        if (_stage == 0) {
            _counter++;
            if (_counter > kFirstStageFrames) {
                _stage++;
                // TODO: Improve move precision (maybe offset error?)
                int diffY = globalBattle->_playerSpr._wy + (9 << 8) / 2 - ((kPelletY + kPelletMoveY) << 8) - (4 << 8);
                int ySteps = (diffY << 8) / kPelletSpeedY;
                for (int i = 0; i < 5; i++) {
                    auto pellet = _pelletSpr[i];
                    _pelletVecX[i] = ((globalBattle->_playerSpr._wx + (9 << 8) / 2 - pellet._wx - (4 << 8)) << 8) / ySteps;
                }
            } else {
                for (auto & pellet : _pelletSpr) {
                    pellet._wy = (kPelletY << 8) + ((kPelletMoveY * _counter) << 8 / kFirstStageFrames);
                }
            }
        } else {
            for (int i = 0; i < 5; i++) {
                auto pellet = _pelletSpr[i];
                pellet._wx += _pelletVecX[i];
                pellet._wy += kPelletSpeedY;
                if (distSquared_fp(pellet._wx + (4 << 8),
                                   pellet._wy + (4 << 8),
                                   globalBattle->_playerSpr._wx + (9 << 8) / 2,
                                   globalBattle->_playerSpr._wy + (9 << 8) / 2) <= (kPelletRadius * kPelletRadius) << 8) {
                    globalBattle->_hitFlag = true;
                    globalSave.hp = 1;
                    return true;
                }
            }
            if (_pelletSpr[0]._wy > 180 << 8) {
                globalBattle->_hitFlag = false;
                return true;
            }
        }
        return false;
    }
}