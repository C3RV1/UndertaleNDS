//
// Created by cervi on 21/04/2025
//

#ifndef FROGGIT_HPP
#define FROGGIT_HPP

#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include "Path.hpp"
#include <memory>

enum class FroggitCommands { ENABLE_HEAD_BOB = 0, DISABLE_HEAD_BOB = 1 };
enum class FroggitActs { CHECK = 0, COMPLIMENT = 1, THREATEN = 2 };

class Froggit : public Enemy {
public:
  Froggit(bool isFirstEnemy);
  void update() final;
  bool damageAnimation() final;
  bool canBeSpared() final;
  void doAct(int actId) final;
  void enemyCommand(u8 command);

  std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) final;

protected:
  void slashFinished() final;
  void damageAnimationFinished() final;
  void shakeSprites(s32 dx) final;

private:
  static constexpr int kX = 27, kY = 93;
  std::shared_ptr<Engine::Sprite> _headSpr, _legsSpr;

  static constexpr s32 kPathOffX = 0, kPathOffY = 4 << 8;
  static constexpr s32 kDeathOffY = 4 << 8;
  static constexpr s32 kPathSpeed = (8 << 8) / 60;
  Path _headPath{{{-(4 << 8) + kPathOffX, 0 + kPathOffY},
                  {0 + kPathOffX, -(2 << 8) + kPathOffY},
                  {(4 << 8) + kPathOffX, 0 + kPathOffY},
                  {0 + kPathOffX, (2 << 8) + kPathOffY},
                  {0 + kPathOffX, -(4 << 8) + kPathOffY}},
                 true,
                 true};
  bool _headBobEnabled = true;
  bool _canBeSpared = false;
};

#endif
