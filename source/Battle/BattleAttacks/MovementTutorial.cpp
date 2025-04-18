//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include <memory>

namespace BtlAttacks {
MovementTutorial::MovementTutorial() {
  _tutorialSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_tutorialSpr, "cutscene/0/spr_guidearrows");
  Engine::spriteSetShown(_tutorialSpr, true);
  _tutorialSpr->_wx = globalBattle->_playerSpr->_wx - (10 << 8);
  _tutorialSpr->_wy = globalBattle->_playerSpr->_wy - (10 << 8);
  _tutorialSpr->_layer = 50;
  int animId = _tutorialSpr->nameToAnimId("tutorial");
  _tutorialSpr->setAnimation(animId);
}

bool MovementTutorial::update() {
  if (keysDown() & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
    return true;
  return false;
}
} // namespace BtlAttacks
