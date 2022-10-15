//
// Created by cervi on 24/08/2022.
//

#include "Save.hpp"
#include "card.hpp"

SaveData globalSave;

void SaveData::clear(ClearType clearType) {
    if (clearType == INTERNAL_RESET || clearType == PLAYER_TRUE_RESET) {
        memset(name, 0, MAX_NAME_LEN + 1);
        saveExists = false;
        memset(flags, 0, 2 * FLAG_COUNT);
        hp = 20; maxHp = 20;
        lv = 1; exp = 0; gold = 0;
        memset(items, 0, ITEM_COUNT + 1);
        lastSavedRoom = 0;
    } else if (clearType == PLAYER_RESET) {
        memset(flags, 0, 2 * 240);  // reset all but persistent flags
        hp = 20; maxHp = 20;
        lv = 1; exp = 0; gold = 0;
        memset(items, 0, ITEM_COUNT + 1);
        lastSavedRoom = 0;
    }
}

void SaveData::loadData() {
    char header[4];
    char expectedHeader[4] = {'U', 'S', 'A', 'V'};

    fCard.seek(0, SEEK_SET);
    fCard.read(header, 4);

    if (memcmp(header, expectedHeader, 4) != 0) {
        clear(INTERNAL_RESET);
        return;
    }

    u32 saveVersion_;
    fCard.read(&saveVersion_, 4);
    if (saveVersion_ != saveVersion) {
        clear(INTERNAL_RESET);
        return;
    }
    fCard.read(name, MAX_NAME_LEN + 1);
    fCard.read(flags, 2 * FLAG_COUNT);
    fCard.read(&maxHp, 1);
    hp = maxHp;
    fCard.read(&lv, 2);
    fCard.read(&exp, 2);
    fCard.read(&gold, 2);
    fCard.read(items, ITEM_COUNT);
    fCard.read(cell, CELL_COUNT);
    items[ITEM_COUNT] = 0;
    cell[CELL_COUNT] = 0;

    saveExists = true;

    fCard.read(&lastSavedRoom, 2);
}

void SaveData::saveData(u16 roomId) {
    char header[4] = {'U', 'S', 'A', 'V'};

    fCard.seek(0, SEEK_SET);
    fCard.write(header, 4);
    u32 saveVersion_ = saveVersion;
    fCard.write(&saveVersion_, 4);
    fCard.write(name, MAX_NAME_LEN + 1);
    fCard.write(flags, 2 * FLAG_COUNT);
    fCard.write(&maxHp, 1);
    fCard.write(&lv, 2);
    fCard.write(&exp, 2);
    fCard.write(&gold, 2);
    fCard.write(items, ITEM_COUNT);
    fCard.write(cell, CELL_COUNT);
    items[ITEM_COUNT] = 0;

    lastSavedRoom = roomId;
    fCard.write(&lastSavedRoom, 2);

    saveExists = true;
}
