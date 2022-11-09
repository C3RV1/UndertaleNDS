//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Formats/utils.hpp"

void Enemy::readFromStream(FILE *f) {
    fread(&_enemyId, 2, 1, f);
    fread(&_maxHp, 2, 1, f);
    _hp = _maxHp;
    fread(&_attackId, 2, 1, f);

    char buffer[100];
    sprintf(buffer, "nitro:/data/enemies/name%d.txt", _enemyId);
    FILE* enemyNameFile = fopen(buffer, "rb");
    if (enemyNameFile) {
        int len = str_len_file(enemyNameFile, '\n');
        fread(_enemyName, len + 1, 1, enemyNameFile);
        _enemyName[len] = '\0';
    } else {
        sprintf(buffer, "Error opening enemy name %d", _enemyId);
        nocashMessage(buffer);
    }
    fclose(enemyNameFile);

    u16 actTextId = 0;
    fread(&actTextId, 2, 1, f);
    fread(&_actOptionCount, 1, 1, f);
    fread(&_spareValue, 1, 1, f);
    fread(&_goldOnWin, 1, 1, f);
    fread(&_expOnKill, 1, 1, f);
    fread(&_defense, 2, 1, f);
    loadActText(actTextId);
}

void Enemy::loadActText(int textId) {
    char buffer[100];
    sprintf(buffer, "nitro:/data/battle_act_txt/%d.txt", textId);
    FILE* actTextFile = fopen(buffer, "rb");
    if (actTextFile) {
        int len = str_len_file(actTextFile, '@');
        delete[] _actText;
        _actText = new char[len + 1];
        fread(_actText, len + 1, 1, actTextFile);
        _actText[len] = '\0';
    } else {
        sprintf(buffer, "Error opening battle act %d", textId);
        nocashMessage(buffer);
    }
    fclose(actTextFile);
}

void Enemy::free_() {
    delete[] _actText;
    _actText = nullptr;
}
