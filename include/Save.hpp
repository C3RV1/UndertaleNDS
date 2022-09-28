//
// Created by cervi on 24/08/2022.
//

#ifndef UNDERTALE_SAVE_HPP
#define UNDERTALE_SAVE_HPP

#define FLAG_COUNT 256
#define ITEM_COUNT 8

#include <stdio.h>
#define ARM9
#include <nds.h>

const int MAX_NAME_LEN = 10;

class SaveData {
public:
    void clear();
    void loadData();
    void saveData(u16 roomId);

    bool saveExists = false;
    char name[MAX_NAME_LEN + 1] = {0};
    u16 flags[FLAG_COUNT] = {0};
    u8 hp = 20, maxHp = 20;
    u8 lv = 1, exp = 0;
    u8 items[ITEM_COUNT + 1] = {0};  // Terminal 0

    u16 lastSavedRoom = 0;
};

extern SaveData globalSave;

#endif //UNDERTALE_SAVE_HPP
