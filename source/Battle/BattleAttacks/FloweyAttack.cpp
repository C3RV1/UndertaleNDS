//
// Created by cervi on 10/09/2022.
//

#include "Battle/BattleAttacks/FloweyAttack.hpp"
#include "Battle/Battle.hpp"
#include "Battle/BattleAttack.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/WAV.hpp"
#include "Engine/math.hpp"
#include "Save.hpp"
#include <memory>

namespace BtlAttacks {
FloweyAttack::FloweyAttack(Battle *battle) : BattleAttack(battle) {
  _hurtSnd = std::make_shared<Audio2::WAV>();
  _hurtSnd->load("snd_hurt1.wav");

  int x = kPelletX;
  for (auto &pellet : _pelletSpr) {
    pellet = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
    Engine::spriteLoadTexture(pellet, "battle/attack_pellets");
    pellet->_wx = x << 8;
    pellet->_wy = kPelletY << 8;
    x += kPelletSpacing;
    Engine::spriteSetShown(pellet, true);
  }
}

bool FloweyAttack::update() {
  if (_stage == 0) {
    _counter++;
    if (_counter > kFirstStageFrames) {
      _stage++;
      // TODO: Improve move precision (maybe offset error?)
      int diffY = _battle->_playerSpr->_wy + (9 << 8) / 2 -
                  ((kPelletY + kPelletMoveY) << 8) + (4 << 8);
      int ySteps = (diffY << 8) / kPelletSpeedY;
      for (int i = 0; i < 5; i++) {
        auto &pellet = _pelletSpr[i];
        int diffX = ((_battle->_playerSpr->_wx + (9 << 8) / 2 -
                      pellet->_wx + (4 << 8))
                     << 8);
        _pelletVecX[i] = diffX / ySteps;
      }
    } else {
      for (auto &pellet : _pelletSpr) {
        pellet->_wy = (kPelletY << 8) +
                      ((kPelletMoveY * _counter) << 8 / kFirstStageFrames);
      }
    }
  } else {
    for (int i = 0; i < 5; i++) {
      auto &pellet = _pelletSpr[i];
      pellet->_wx += _pelletVecX[i];
      pellet->_wy += kPelletSpeedY;
      if (distSquared_fp(pellet->_wx + (4 << 8), pellet->_wy + (4 << 8),
                         _battle->_playerSpr->_wx + (9 << 8) / 2,
                         _battle->_playerSpr->_wy + (9 << 8) / 2) <=
          (kPelletRadius * kPelletRadius) << 8) {
        _battle->_save->flags[kFlagAttack] = 1;
        _battle->_save->hp = 1;
        _battle->showHp();
        Audio2::audioManager.play(std::move(_hurtSnd));
        return true;
      }
    }
    if (_pelletSpr[0]->_wy > 180 << 8) {
      _battle->_save->flags[kFlagAttack] = 0;
      return true;
    }
  }
  return false;
}
} // namespace BtlAttacks
