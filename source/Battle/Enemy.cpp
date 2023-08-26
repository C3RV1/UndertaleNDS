//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Formats/utils.hpp"
#include "Engine/Engine.hpp"

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
        _enemyName.resize(len);
        fread(&_enemyName[0], len, 1, enemyNameFile);
        fseek(enemyNameFile, 1, SEEK_CUR);
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
    std::string buffer = "nitro:/data/battle_act_txt/" + std::to_string(textId) + ".txt";
    FILE* actTextFile = fopen(buffer.c_str(), "rb");
    if (actTextFile) {
        int len = str_len_file(actTextFile, '@');
        _actText.resize(len);
        fread(&_actText[0], len, 1, actTextFile);
        fseek(actTextFile, 1, SEEK_CUR);
    } else {
        buffer = "Error opening battle act " + std::to_string(textId);
        Engine::throw_(buffer);
    }
    fclose(actTextFile);
}
