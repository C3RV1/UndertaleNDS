//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Texture.hpp"
#include <memory>

namespace BtlAttacks {
MovementTutorial::MovementTutorial() : _tutorialSpr(Engine::Allocated3D) {
  _tutorialSpr.loadTexture(
      Engine::textureManager.loadTexture("cutscene/0/spr_guidearrows"));
  _tutorialSpr.setShown(true);
  _tutorialSpr._wx = globalBattle->_playerSpr._wx - (10 << 8);
  _tutorialSpr._wy = globalBattle->_playerSpr._wy - (10 << 8);
  _tutorialSpr._layer = 50;
  int animId = _tutorialSpr.nameToAnimId("tutorial");
  _tutorialSpr.setAnimation(animId);
}

bool MovementTutorial::update() {
  if (keysDown() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
    return true;
  return false;
}
} // namespace BtlAttacks
