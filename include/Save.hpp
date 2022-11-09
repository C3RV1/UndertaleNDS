//
// Created by cervi on 24/08/2022.
//

#ifndef UNDERTALE_SAVE_HPP
#define UNDERTALE_SAVE_HPP

#define FLAG_COUNT 256
#define ITEM_COUNT 8
#define CELL_COUNT 8

#include <cstdio>
#define ARM9
#include <nds.h>

const int MAX_NAME_LEN = 10;

enum ClearType {
    INTERNAL_RESET,
    PLAYER_RESET,
    PLAYER_TRUE_RESET
};

enum Items {
    MONSTER_CANDY = 1,
    CROQUET_ROLL,
    STICK,
    BANDAGE
};

class SaveData {
public:
    const u32 saveVersion = 5;
    void clear(ClearType clearType);
    void loadData();
    void saveData(u16 roomId);

    bool saveExists = false;
    char name[MAX_NAME_LEN + 1] = {0};
    u16 flags[FLAG_COUNT] = {0};
    u8 hp = 20, maxHp = 20;
    u16 lv = 1, exp = 0, gold = 0;
    u8 items[ITEM_COUNT + 1] = {0};  // Terminal 0
    u8 cell[CELL_COUNT + 1] = {0};  // Terminal 0
    u8 cWeapon = Items::STICK; // Starting: stick
    u8 cArmor = Items::BANDAGE; // Starting: bandage

    u16 lastSavedRoom = 0;
};

extern SaveData globalSave;

#endif //UNDERTALE_SAVE_HPP
