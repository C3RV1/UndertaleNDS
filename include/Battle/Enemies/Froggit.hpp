//
// Created by cervi on 21/04/2025
//

#ifndef FROGGIT_HPP
#define FROGGIT_HPP

#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include "Path.hpp"
#include <memory>

class Froggit : public Enemy {
public:
  Froggit(bool isFirstEnemy);
  void update() final;
  bool damageAnimation() final;
  bool canBeSpared() final;
  void doAct(int actId);

  std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) final;

protected:
  void slashFinished() final;
  void damageAnimationFinished() final;

private:
  static constexpr int kX = 27, kY = 93;
  std::shared_ptr<Engine::Sprite> _headSpr, _legsSpr;

  static constexpr s32 kPathOffX = 0, kPathOffY = 4 << 8;
  Path _headPath{{{-(4 << 8) + kPathOffX, 0 + kPathOffY},
                  {0 + kPathOffX, -(2 << 8) + kPathOffY},
                  {(4 << 8) + kPathOffX, 0 + kPathOffY},
                  {0 + kPathOffX, (2 << 8) + kPathOffY},
                  {0 + kPathOffX, -(4 << 8) + kPathOffY}},
                 true,
                 true};
  static constexpr s32 kPathSpeed = (8 << 8) / 60;
};

#endif
