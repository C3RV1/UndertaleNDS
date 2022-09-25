//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLEATTACK_HPP
#define UNDERTALE_BATTLEATTACK_HPP

#include <stdint.h>

class BattleAttack {
public:
    virtual bool update() {return true;};
    virtual void draw() {};
    virtual ~BattleAttack() = default;
};

BattleAttack* getBattleAttack(uint16_t attackId);

#endif //UNDERTALE_BATTLEATTACK_HPP
