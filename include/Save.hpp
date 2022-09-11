//
// Created by cervi on 24/08/2022.
//

#ifndef UNDERTALE_SAVE_HPP
#define UNDERTALE_SAVE_HPP

#define FLAG_COUNT 256
#define ITEM_COUNT 8

#include <stdint.h>

enum Flags {
    RUIN_PROGRESS = 0
};

struct SaveData {
    char* name = nullptr;
    uint16_t flags[FLAG_COUNT] = {0};
    uint8_t hp = 20, maxHp = 20;
    uint8_t lv = 1, exp = 0;
    uint8_t items[ITEM_COUNT + 1] = {0};  // Terminal 0
};

extern SaveData globalSave;

#endif //UNDERTALE_SAVE_HPP
