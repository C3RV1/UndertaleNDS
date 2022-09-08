//
// Created by cervi on 08/09/2022.
//
#include "Battle/BattleAttack.hpp"

BattleAttack* getBattleAttack(uint16_t attackId) {
    BattleAttack* btlAtk = nullptr;

    switch (attackId) {
        case 1:
            btlAtk = new BtlAttacks::MovementTutorial();
            break;
    }

    if (btlAtk != nullptr) {
        btlAtk->load();
    }
    return btlAtk;
}