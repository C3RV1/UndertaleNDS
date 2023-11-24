//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttack.hpp"
#include "Battle/BattleAttacks/MovementTutorial.hpp"
#include "Battle/BattleAttacks/FloweyAttack.hpp"
#include "Battle/BattleAttacks/FloweyAttack2.hpp"

std::unique_ptr<BattleAttack> getBattleAttack(u16 attackId) {
    switch (attackId) {
        case 1:
            return std::make_unique<BtlAttacks::MovementTutorial>();
        case 2:
            return std::make_unique<BtlAttacks::FloweyAttack>();
        case 3:
            return std::make_unique<BtlAttacks::FloweyAttack2>();
        default:
            return nullptr;
    }
}