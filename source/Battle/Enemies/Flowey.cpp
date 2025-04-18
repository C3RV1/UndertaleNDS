#include "Battle/Enemies/Flowey.hpp"
#include "Battle/BattleAttack.hpp"
#include "Battle/BattleAttacks/FloweyAttack.hpp"
#include "Battle/BattleAttacks/FloweyAttack2.hpp"
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include <memory>

Flowey::Flowey() {
  _hp = 100;
  floweySpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  Engine::spriteLoadTexture(floweySpr, "speaker/flowey");
  floweySpr->_wx = 30 << 8;
  floweySpr->_wy = ((192 - 44) / 2) << 8;
  floweySpr->_layer = 1;
  Engine::spriteSetShown(floweySpr, true);
}

std::shared_ptr<Engine::Sprite> Flowey::getSprite(u8 spriteId) {
  return floweySpr;
}

std::unique_ptr<BattleAttack> Flowey::getBattleAttack() {
  switch (floweyAttack) {
  case FloweyCommands::MOVEMENT:
    return std::make_unique<BtlAttacks::MovementTutorial>();
  case FloweyCommands::PROGRESS_TO_ATTACK:
    return std::make_unique<BtlAttacks::FloweyAttack>();
  case FloweyCommands::PROGRESS_TO_KILL:
    return std::make_unique<BtlAttacks::FloweyAttack2>();
  default:
    return nullptr;
  }
}

void Flowey::enemyCommand(u8 command) {
  floweyAttack = static_cast<FloweyCommands>(command);
}
