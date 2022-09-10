//
// Created by cervi on 02/09/2022.
//

#ifndef LAYTON_BATTLEATTACK_HPP
#define LAYTON_BATTLEATTACK_HPP

class BattleAttack;

#include <stdint.h>

class BattleAttack {
public:
    virtual bool update() {return true;};
    virtual void draw() {};
    virtual ~BattleAttack() = default;
};

#include "BattleAttacks/MovementTutorial.hpp"
#include "BattleAttacks/FloweyAttack.hpp"

BattleAttack* getBattleAttack(uint16_t attackId);

#endif //LAYTON_BATTLEATTACK_HPP
