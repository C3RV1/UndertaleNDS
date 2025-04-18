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

class Enemy {
public:
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
  }
  virtual void doAct(int actId) {}
  virtual bool damageAnimation() { return true; }
  virtual bool canBeSpared() { return true; }
  virtual Engine::Sprite *getSprite(u8 spriteId) { return nullptr; }
  virtual std::unique_ptr<BattleAttack> getBattleAttack() { return nullptr; }
  virtual void enemyCommand(u8 command) {}
  virtual ~Enemy(){};

  int _hp = 0;
  int _maxHp = 0;
  u8 _goldOnWin = 0;
  u8 _expOnKill = 0;

protected:
  void renderHealth(int x, int y);
  void loadName(int enemyId);
  void loadActText(int textId);

  std::string _actText;
  u8 _actOptionCount = 0;
  u8 _defense = 0;
  std::string _name;
  int _prevHp;
  bool _spared = false;
};

std::unique_ptr<Enemy> getEnemy(u16 enemyId);

#endif // UNDERTALE_ENEMY_HPP
