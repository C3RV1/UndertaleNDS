//
// Created by cervi on 21/04/2025
//

#ifndef FROGGIT_HPP
#define FROGGIT_HPP

#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include <memory>

class Froggit : public Enemy {
public:
  Froggit(bool isFirstEnemy);
  void update() final;
  bool damageAnimation() final;
  bool canBeSpared() final;

  std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) final;

protected:
  void slashFinished() final;
  void damageAnimationFinished() final;

private:
  static constexpr int kX = 27, kY = 93;
  static constexpr int kAnimFrames = 180;
  int _stretchFrames = 0;
  std::shared_ptr<Engine::Sprite> _headSpr, _legsSpr;
};

#endif
