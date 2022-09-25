//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttack.hpp"
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Battle/BattleAttacks/FloweyAttack.hpp"
#include "Battle/BattleAttacks/FloweyAttack2.hpp"

BattleAttack* getBattleAttack(uint16_t attackId) {
    switch (attackId) {
        case 1:
            return new BtlAttacks::MovementTutorial();
        case 2:
            return new BtlAttacks::FloweyAttack();
        case 3:
            return new BtlAttacks::FloweyAttack2();
        default:
            return nullptr;
    }
}