//
// Created by cervi on 24/08/2022.
//

#include "Save.hpp"
#include "card.hpp"

SaveData globalSave;

void SaveData::clear() {
    memset(name, 0, MAX_NAME_LEN + 1);
    saveExists = false;
    memset(flags, 0, 2 * FLAG_COUNT);
    hp = 20; maxHp = 20;
    lv = 1; exp = 0;
    memset(items, 0, ITEM_COUNT + 1);
}

void SaveData::loadData() {
    char header[4];
    char expectedHeader[4] = {'U', 'S', 'A', 'V'};

    fCard.seek(0, SEEK_SET);
    fCard.read(header, 4);

    if (memcmp(header, expectedHeader, 4) != 0) {
        clear();
        return;
    }

    fCard.read(name, MAX_NAME_LEN + 1);
    fCard.read(flags, 2 * FLAG_COUNT);
    fCard.read(&hp, 1);
    fCard.read(&maxHp, 1);
    fCard.read(&lv, 1);
    fCard.read(&exp, 1);
    fCard.read(items, ITEM_COUNT + 1);

    saveExists = true;
}

void SaveData::saveData() {
    char header[4] = {'U', 'S', 'A', 'V'};

    fCard.seek(0, SEEK_SET);
    fCard.write(header, 4);
    fCard.write(name, MAX_NAME_LEN + 1);
    fCard.write(flags, 2 * FLAG_COUNT);
    fCard.write(&hp, 1);
    fCard.write(&maxHp, 1);
    fCard.write(&lv, 1);
    fCard.write(&exp, 1);
    fCard.write(items, ITEM_COUNT + 1);
}
