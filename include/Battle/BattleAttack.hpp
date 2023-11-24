//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLE_ATTACK_HPP
#define UNDERTALE_BATTLE_ATTACK_HPP

#include <nds.h>
#include <memory>

class BattleAttack {
public:
    virtual bool update() {return true;};
    virtual ~BattleAttack() = default;
};

std::unique_ptr<BattleAttack> getBattleAttack(u16 attackId);

#endif //UNDERTALE_BATTLE_ATTACK_HPP
