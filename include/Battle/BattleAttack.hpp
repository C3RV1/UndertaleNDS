//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLEATTACK_HPP
#define UNDERTALE_BATTLEATTACK_HPP

#define ARM9
#include <nds.h>

class BattleAttack {
public:
    virtual bool update() {return true;};
    virtual ~BattleAttack() = default;
};

BattleAttack* getBattleAttack(u16 attackId);

#endif //UNDERTALE_BATTLEATTACK_HPP
