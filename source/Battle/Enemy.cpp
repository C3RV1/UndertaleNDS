//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Formats/utils.hpp"

void Enemy::readFromStream(FILE *f) {
    fread(&enemyId, 2, 1, f);
    fread(&maxHp, 2, 1, f);
    hp = maxHp;
    fread(&attackId, 2, 1, f);

    char buffer[100];
    sprintf(buffer, "nitro:/data/enemies/name%d.txt", enemyId);
    FILE* enemyNameFile = fopen(buffer, "rb");
    if (enemyNameFile) {
        int len = str_len_file(enemyNameFile, '\n');
        fread(enemyName, len + 1, 1, enemyNameFile);
        enemyName[len] = '\0';
    } else {
        sprintf(buffer, "Error opening enemy name %d", enemyId);
        nocashMessage(buffer);
    }
    fclose(enemyNameFile);

    u16 actTextId = 0;
    fread(&actTextId, 2, 1, f);
    fread(&actOptionCount, 1, 1, f);
    fread(&spareValue, 1, 1, f);
    fread(&goldOnWin, 1, 1, f);
    fread(&expOnKill, 1, 1, f);
    fread(&defense, 2, 1, f);
    loadActText(actTextId);
}

void Enemy::loadActText(int textId) {
    char buffer[100];
    sprintf(buffer, "nitro:/data/battle_act_txt/%d.txt", textId);
    FILE* actTextFile = fopen(buffer, "rb");
    if (actTextFile) {
        int len = str_len_file(actTextFile, '@');
        delete[] actText;
        actText = new char[len + 1];
        fread(actText, len + 1, 1, actTextFile);
        actText[len] = '\0';
    } else {
        sprintf(buffer, "Error opening battle act %d", textId);
        nocashMessage(buffer);
    }
    fclose(actTextFile);
}

void Enemy::free_() {
    delete[] actText;
    actText = nullptr;
}
