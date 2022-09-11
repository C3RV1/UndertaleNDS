//
// Created by cervi on 02/09/2022.
//

#ifndef UNDERTALE_BATTLEATTACK_HPP
#define UNDERTALE_BATTLEATTACK_HPP

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
#include "BattleAttacks/FloweyAttack2.hpp"

BattleAttack* getBattleAttack(uint16_t attackId);

#endif //UNDERTALE_BATTLEATTACK_HPP
