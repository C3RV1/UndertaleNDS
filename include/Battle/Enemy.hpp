//
// Created by cervi on 02/10/2022.
//

#ifndef UNDERTALE_ENEMY_HPP
#define UNDERTALE_ENEMY_HPP

#include "Battle/BattleAttack.hpp"
#include "Engine/Sprite.hpp"
#include <cstdio>
#include <memory>
#include <nds.h>
#include <string>
#include <vector>

enum class EnemyDamageAnimationStep { SLASH = 0, DAMAGE_NUMBERS = 1 };

// TODO: Disintegrate on kill. (How? 3D engine? Particles? Mosaic??)

class Enemy {
public:
  Enemy();
  std::string getName() { return _name; }
  std::string getActText() { return _actText; }
  bool getSpared() { return _spared; }
  u8 getDefense() { return _defense; }
  u8 getActOptionCount() { return _actOptionCount; }
  virtual void update() {}
  virtual void doDamage(int damage) {
    _prevHp = _hp;
    _hp -= damage;
    if (_hp < 0)
      _hp = 0;
    loadDamageSprites(damage);
  }
  virtual void doAct(int actId) {}
  virtual bool damageAnimation() { return true; }
  virtual bool canBeSpared() { return true; }
  virtual std::shared_ptr<Engine::Sprite> getSprite(u8 spriteId) {
    return nullptr;
  }
  virtual std::unique_ptr<BattleAttack> getBattleAttack() { return nullptr; }
  virtual void enemyCommand(u8 command) {}
  virtual ~Enemy(){};

  int _hp = 0;
  int _maxHp = 0;
  u8 _goldOnWin = 0;
  u8 _expOnKill = 0;

protected:
  virtual void slashFinished();
  virtual void damageAnimationFinished() {}
  bool defaultDamageAnimation(s32 x, s32 y, int width, int height);
  void loadDamageSprites(int damage);
  void doSlash(s32 x, s32 y, int counter, int maxCounter);
  void doDamageNumbers(s32 x, s32 y, int counter, int maxCounter);
  void doRenderHealth(int x, int y, int counter, int maxCounter);
  void doShake(int counter, int maxCounter);
  virtual void shakeSprites(s32 dx) {}
  void loadName(int enemyId);
  void loadActText(int textId);

  std::string _name;
  std::string _actText;
  u8 _actOptionCount = 0;
  u8 _defense = 0;
  int _prevHp;
  bool _spared = false;

private:
  // If we want to have the peak at t=0.5,
  // vo = -a/2
  // Then the max deltaY = -a/8
  u8 _damageSpriteCounter;
  EnemyDamageAnimationStep _damageAnimStep;
  std::shared_ptr<Engine::Sprite> _slashSpr;
  std::vector<std::shared_ptr<Engine::Sprite>> _damageNumbersSpr;
  bool _missed;

  static constexpr s32 despY = -(10 << 8); // Maximum displacement in y axis.
  static constexpr s32 kDamageNumAccY = -despY * 8;
  static constexpr s32 kDamageNumSpeedY = -kDamageNumAccY / 2;

  static constexpr int kHpBarWidth = 30;
  static constexpr int kHpBarHeigth = 6;

  static constexpr int kSlashFrames = 10 * 6;
  static constexpr int kDamageNumFrames = 90;

  static constexpr s32 kShakeMaxAngle = DEGREES_IN_CIRCLE * 10;
  static constexpr s32 kShakeAmplitude = 5 << 8;
};

std::unique_ptr<Enemy> getEnemy(u16 enemyId);

#endif // UNDERTALE_ENEMY_HPP
