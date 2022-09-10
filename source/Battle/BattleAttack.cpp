//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttack.hpp"

BattleAttack* getBattleAttack(uint16_t attackId) {
    switch (attackId) {
        case 1:
            return new BtlAttacks::MovementTutorial();
        case 2:
            return new BtlAttacks::FloweyAttack();
        default:
            return nullptr;
    }
}