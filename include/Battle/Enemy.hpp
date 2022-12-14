//
// Created by cervi on 02/10/2022.
//

#ifndef UNDERTALE_ENEMY_HPP
#define UNDERTALE_ENEMY_HPP

#define ARM9
#include <nds.h>
#include <cstdio>

class Enemy {
public:
    void readFromStream(FILE *f);
    void free_();
    void loadActText(int textId);
    u16 _enemyId = 0;
    char _enemyName[20] = {0};
    u16 _hp = 0;
    u16 _maxHp = 0;
    char *_actText = nullptr;
    u8 _actOptionCount = 0;
    u16 _attackId = 0;
    u8 _spareValue = 0;  // When it reaches 100, enemy can be spared
    u8 _goldOnWin = 0;
    u8 _expOnKill = 0;
    s16 _defense = 0;
    bool _spared = false;
};

#endif //UNDERTALE_ENEMY_HPP
