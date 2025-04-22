//
// Created by cervi on 17/04/2025
//

#ifndef DUMMY_HPP
#define DUMMY_HPP

#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include <memory>

class Dummy : public Enemy {
public:
  Dummy();
  bool damageAnimation() final;
  bool canBeSpared() final;
  std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) final;

protected:
  void slashFinished() final;
  void shakeSprites(s32 dx) final;

private:
  static constexpr int kX = 30, kY = (192 - 52) / 2;
  std::shared_ptr<Engine::Sprite> _dummySpr;
};

#endif
