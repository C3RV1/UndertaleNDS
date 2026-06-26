//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLE_ATTACK_HPP
#define UNDERTALE_BATTLE_ATTACK_HPP

#include <memory>
#include <nds.h>

class Battle;

class BattleAttack {
public:
  explicit BattleAttack(Battle *battle) : _battle(battle) {}
  virtual bool update() { return true; };
  virtual ~BattleAttack() = default;
protected:
  Battle* _battle;
};

#endif // UNDERTALE_BATTLE_ATTACK_HPP
