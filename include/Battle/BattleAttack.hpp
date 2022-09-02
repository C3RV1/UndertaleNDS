//
// Created by cervi on 02/09/2022.
//

#ifndef LAYTON_BATTLEATTACK_HPP
#define LAYTON_BATTLEATTACK_HPP

#include <stdint.h>

class BattleAttack {
public:
    virtual void load() {};
    virtual void free_() {};
    virtual bool update() {return true;};
    virtual void draw() {};
    virtual ~BattleAttack() = default;
};

void getBattleAttack(uint16_t attackId);

#endif //LAYTON_BATTLEATTACK_HPP
