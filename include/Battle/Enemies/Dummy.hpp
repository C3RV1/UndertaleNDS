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
  static constexpr int kX = 30, kY = (192 - 52) / 2;
  Engine::Sprite _dummySpr{Engine::AllocationMode::AllocatedOAM};
};

#endif
