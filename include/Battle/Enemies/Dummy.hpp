//
// Created by cervi on 17/04/2025
//

#ifndef DUMMY_HPP
#define DUMMY_HPP

#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"

class Dummy : public Enemy {
public:
  Dummy();
  void doDamage(int damage) final;
  void doAct(int actId) final;
  bool damageAnimation() final;
  bool canBeSpared() final;
  Engine::Sprite *getSprite(u8 spriteId) final;

private:
  Engine::Sprite dummySpr{Engine::AllocationMode::AllocatedOAM};
};

#endif
