//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLE_ATTACK_HPP
#define UNDERTALE_BATTLE_ATTACK_HPP

#define ARM9
#include <nds.h>

class BattleAttack {
public:
    virtual bool update() {return true;};
    virtual ~BattleAttack() = default;
};

BattleAttack* getBattleAttack(u16 attackId);

#endif //UNDERTALE_BATTLE_ATTACK_HPP
