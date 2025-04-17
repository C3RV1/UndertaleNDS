//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLE_ATTACK_HPP
#define UNDERTALE_BATTLE_ATTACK_HPP

#include <memory>
#include <nds.h>

class BattleAttack {
public:
  virtual bool update() { return true; };
  virtual ~BattleAttack() = default;
};

#endif // UNDERTALE_BATTLE_ATTACK_HPP
