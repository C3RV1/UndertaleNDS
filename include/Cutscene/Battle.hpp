//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_BATTLE_HPP
#define LAYTON_BATTLE_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>

struct Enemy {
    uint16_t enemyId;
    char* enemyName;
    uint16_t hp;
    uint16_t maxHp;
};

class Battle {
public:
    void loadFromStream(FILE* stream);
private:
    uint8_t enemyCount;
    Enemy* enemies;
};

void startBattle(FILE* stream);

#endif //LAYTON_BATTLE_HPP
