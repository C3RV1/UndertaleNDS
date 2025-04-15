//
// Created by cervi on 24/08/2022.
//

#include "Save.hpp"
#include "DEBUG_FLAGS.hpp"
#include "card.hpp"

SaveData globalSave;

void SaveData::clear(ClearType clearType) {
  lv = 1;
  exp = 0;
  gold = 0;
  hp = 20;
  maxHp = 20;
  memset(items, 0, ITEM_COUNT + 1);
  lastSavedRoom = 0;
  if (clearType == INTERNAL_RESET || clearType == PLAYER_TRUE_RESET) {
    memset(name, 0, MAX_NAME_LEN + 1);
    saveExists = false;
    memset(flags, 0, 2 * FLAG_COUNT);
    cWeapon = Items::STICK;
    cArmor = Items::BANDAGE;
  } else if (clearType == PLAYER_RESET) {
    memset(flags, 0, 2 * FlagIds::PERSISTENT); // reset all but persistent flags
  }
}

#include "Engine/Engine.hpp"

void SaveData::loadData() {
  // Initializing it like this for some reason fixes the issues
  unsigned char header[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  char expectedHeader[4] = {'U', 'S', 'A', 'V'};

  fCard.open("rb");
  fCard.seek(0, SEEK_SET);
  bool readCorrectly = fCard.read(header, 4);

  if (memcmp(header, expectedHeader, 4) != 0) {
#ifdef DEBUG_SAVE
    nocashMessage("Save: BAD HEADER.");
#endif
    fCard.close();
    clear(INTERNAL_RESET);
    return;
  }

  u32 saveVersion_;
  readCorrectly &= fCard.read(&saveVersion_, 4);
  if (saveVersion_ != saveVersion) {
#ifdef DEBUG_SAVE
    nocashMessage("Save: BAD VERSION.");
#endif
    fCard.close();
    clear(INTERNAL_RESET);
    return;
  }
  readCorrectly &= fCard.read(name, MAX_NAME_LEN + 1);
  readCorrectly &= fCard.read(flags, 2 * FLAG_COUNT);
  readCorrectly &= fCard.read(&maxHp, 1);
  hp = maxHp;
  readCorrectly &= fCard.read(&lv, 2);
  readCorrectly &= fCard.read(&exp, 2);
  readCorrectly &= fCard.read(&gold, 2);
  readCorrectly &= fCard.read(items, ITEM_COUNT);
  readCorrectly &= fCard.read(cell, CELL_COUNT);
  items[ITEM_COUNT] = 0;
  cell[CELL_COUNT] = 0;
  readCorrectly &= fCard.read(&cWeapon, 1);
  readCorrectly &= fCard.read(&cArmor, 1);

  saveExists = true;

  readCorrectly &= fCard.read(&lastSavedRoom, 2);
  fCard.close();

  if (!readCorrectly) {
    clear(INTERNAL_RESET);
    return;
  }
}

bool SaveData::saveData(u16 roomId) {
  char header[4] = {'U', 'S', 'A', 'V'};

  fCard.open("wb");
  fCard.seek(0, SEEK_SET);
  bool writeCorrectly = fCard.write(header, 4);
  u32 saveVersion_ = saveVersion;
  writeCorrectly &= fCard.write(&saveVersion_, 4);
  writeCorrectly &= fCard.write(name, MAX_NAME_LEN + 1);
  writeCorrectly &= fCard.write(flags, 2 * FLAG_COUNT);
  writeCorrectly &= fCard.write(&maxHp, 1);
  writeCorrectly &= fCard.write(&lv, 2);
  writeCorrectly &= fCard.write(&exp, 2);
  writeCorrectly &= fCard.write(&gold, 2);
  writeCorrectly &= fCard.write(items, ITEM_COUNT);
  writeCorrectly &= fCard.write(cell, CELL_COUNT);
  writeCorrectly &= fCard.write(&cWeapon, 1);
  writeCorrectly &= fCard.write(&cArmor, 1);
  items[ITEM_COUNT] = 0;

  lastSavedRoom = roomId;
  writeCorrectly &= fCard.write(&lastSavedRoom, 2);

  saveExists = true;
  fCard.close();

  return writeCorrectly;
}

void SaveData::writePermanentFlags() {
  fCard.open("wb");
  fCard.seek(4 + 4 + MAX_NAME_LEN + 1 + 2 * FlagIds::PERSISTENT, SEEK_SET);
  fCard.write(&flags[FlagIds::PERSISTENT],
              (FLAG_COUNT - FlagIds::PERSISTENT) * 2);
  fCard.close();
}
