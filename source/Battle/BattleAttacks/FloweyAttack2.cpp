//
// Created by cervi on 10/09/2022.
//

#include "Battle/BattleAttacks/FloweyAttack2.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/math.hpp"
#include "Save.hpp"
#include <memory>

namespace BtlAttacks {
FloweyAttack2::FloweyAttack2() {
  int i = 0;
  for (auto &pellet : _pelletSpr) {
    pellet = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
    Engine::spriteLoadTexture(pellet, "battle/attack_pellets");
    // Set pellets in clockwise order
    // Two rows, one at top and one at bottom
    // Of pellets
    if (i < kPelletW) {
      pellet->_wx = (kPelletX + i * kPelletSpacingX) << 8;
      pellet->_wy = kPelletY << 8;
    } else if (i - kPelletW < kPelletW) {
      int i2 = i - kPelletW;
      pellet->_wx = ((kPelletX + (kPelletW - 1 - i2) * kPelletSpacingX) << 8);
      pellet->_wy = (kPelletY + kPelletSpacingY) << 8;
    }
    Engine::spriteSetShown(pellet, false);
    i++;
  }
}

bool FloweyAttack2::update() {
  if (_stage == 0) {
    _counter++;
    if (_counter < kShowFrames)
      return false;
    _counter = 0;
    Engine::spriteSetShown(_pelletSpr[_cPelletShown], true);
    _cPelletShown++;
    if (_cPelletShown == kPelletW * 2) {
      _stage = 1;
    }
  } else {
    for (int i = 0; i < kPelletW * 2; i++) {
      auto &pellet = _pelletSpr[i];
      if (i < kPelletW) {
        pellet->_wy += kPelletSpeed;
      } else if (i - kPelletW < kPelletW) {
        pellet->_wy -= kPelletSpeed;
      }
      if (distSquared_fp(pellet->_wx + (4 << 8), pellet->_wy + (4 << 8),
                         globalBattle->_playerSpr->_wx + (9 << 8) / 2,
                         globalBattle->_playerSpr->_wy + (9 << 8) / 2) <=
          (kPelletRadius * kPelletRadius) << 8) {
        globalSave.hp = 20;
        return true;
      }
    }
  }
  return false;
}
} // namespace BtlAttacks
