//
// Created by cervi on 17/04/2025
//

#ifndef FLOWEY_HPP
#define FLOWEY_HPP

#include "Battle/BattleAttack.hpp"
#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include <memory>

enum class FloweyCommands {
  MOVEMENT = 0,
  PROGRESS_TO_ATTACK = 1,
  PROGRESS_TO_KILL = 2
};

class Flowey : public Enemy {
public:
  Flowey();
  std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) final;
  std::unique_ptr<BattleAttack> getBattleAttack() final;
  void enemyCommand(u8 command) final;
  void enter() final;

private:
  FloweyCommands floweyAttack = FloweyCommands::MOVEMENT;
  std::shared_ptr<Engine::Sprite> floweySpr;
};

#endif
