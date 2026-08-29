//
// Created by cervi on 28/09/2022.
//

#include "card.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Engine/Engine.hpp"
#include <cerrno>
#include <cstdio>
#include <fat.h>
#include <string>

bool CardBuffer::open(const char *mode) {
  if (_opened)
    return true;

  _pos = 0;
  _opened = true;

  if (access("fat:/", F_OK) == 0) { // check if sd was inited
    debug_save("Running in fat.");

    _fatFile = fopen("fat:/Undertale.sav", mode);

    if (_fatFile)
      return true;
    else
      Engine::log_("SaveFile couldn't be opened. Errno: " +
                   std::to_string(errno));
  }

  if (access("sd:/", F_OK) == 0) {
    debug_save("Running in sd.");
    _fatFile = fopen("sd:/Undertale.sav", mode);
    if (_fatFile)
      return true;
    else
      Engine::log_("SaveFile couldn't be opened. Errno: " +
                   std::to_string(errno));
  }

  _fatFile = nullptr;
  if (cardIsValid()) {
    debug_save("Running in card.");
    return true;
  }

  debug_save("Couldn't open save file.");
  _opened = false;
  return false;
}

void CardBuffer::close() {
  if (!_opened)
    return;
  debug_save("Closing card buffer.");
  if (_fatFile != nullptr) {
    debug_save("Closing savefile buffer.");
    fclose(_fatFile);
    _fatFile = nullptr;
  }
  _opened = false;
}

u8 cardCommand(u8 command, bool hold) {
  REG_AUXSPICNT = 0xa000 + (hold ? 0x40 : 0);
  eepromWaitBusy();
  REG_AUXSPIDATA = command;
  eepromWaitBusy();
  return REG_AUXSPIDATA & 0xFF;
}

u8 cardTransfer(u8 data) {
  REG_AUXSPIDATA = data;
  eepromWaitBusy();
  return REG_AUXSPIDATA & 0xFF;
}

void cardWaitInProgress() {
  cardCommand(SPI_EEPROM_RDSR, true);

  if (cardCommand(0, false) == 0xFF)
    Engine::throw_("Error accessing savefile.");

  do {
    cardCommand(SPI_EEPROM_RDSR, true);
  } while (cardCommand(0, false) & 1);
}

void cardReadBytes(u8 *dst, u32 addr, u16 size) {
  int oldIME = enterCriticalSection();
  bool cardOwner = sysGetCardOwner();
  sysSetCardOwner(BUS_OWNER_ARM9);

  cardCommand(SPI_EEPROM_READ, true);
  cardTransfer(addr >> 8);
  cardTransfer(addr & 0xff);
  for (u16 i = 0; i < size; i++) {
    if (i != size - 1) {
      *dst++ = cardTransfer(0);
    } else {
      *dst++ = cardCommand(0, false);
    }
  }
  cardWaitInProgress();
  cardWaitInProgress();
  swiDelay(14);

  sysSetCardOwner(cardOwner);
  leaveCriticalSection(oldIME);
}

void cardWriteBytes(u8 *src, u32 addr, u16 size) {
  int oldIME = enterCriticalSection();
  bool cardOwner = sysGetCardOwner();
  sysSetCardOwner(BUS_OWNER_ARM9);

  while (size != 0) {
    cardCommand(SPI_EEPROM_WREN, true);
    cardCommand(0, false);

    u16 copySize = 32 - (addr % 32);
    if (copySize > size)
      copySize = size;

    cardCommand(SPI_EEPROM_PP, true);
    cardTransfer(addr >> 8);
    cardTransfer(addr & 0xff);
    for (u16 i = 0; i < copySize; i++) {
      if (i != copySize - 1) {
        cardTransfer(*src++);
      } else {
        cardCommand(*src++, false);
      }
    }

    cardWaitInProgress();
    cardWaitInProgress();
    swiDelay(14);

    size -= copySize;
    addr += copySize;
  }

  sysSetCardOwner(cardOwner);
  leaveCriticalSection(oldIME);
}

bool cardIsValid() { return REG_AUXSPIDATA != 0xFF; }

void CardBuffer::read(void *data, size_t size) {
  if (!_opened)
    return;
  if (_fatFile == nullptr) {
    debug_save("Reading from card. Pos: " + std::to_string(_pos));
    cardReadBytes((u8 *)data, _pos, size);
  } else {
    size_t bytes_read = fread(data, 1, size, _fatFile);

    debug_save("Read " + std::to_string(bytes_read) + " bytes of " +
               std::to_string(size));

    if (bytes_read < size) {
      Engine::log_("ERROR READING. Errno: " + std::to_string(errno));
      return;
    }
  }
  _pos += size;
}

void CardBuffer::write(void *src, size_t size) {
  if (!_opened)
    return;
  if (_fatFile == nullptr) {
    debug_save("Writing " + std::to_string(size) + " bytes to card.");
    cardWriteBytes((u8 *)src, _pos, size);
  } else {
    debug_save("Writing " + std::to_string(size) + " bytes to savefile.");
    size_t bytesWritten = fwrite(src, 1, size, _fatFile);

    debug_save("Written " + std::to_string(bytesWritten) +
               " bytes to savefile.");

    if (bytesWritten < size) {
      Engine::log_("ERROR WRITING. Errno: " + std::to_string(errno));
      return;
    }
  }
  _pos += size;
}

void CardBuffer::seek(s32 offset, u8 mode) {
  if (!_opened)
    return;
  if (mode == SEEK_SET)
    _pos = offset;
  else if (mode == SEEK_CUR)
    _pos += offset;
  else if (mode == SEEK_END)
    _pos = 7999 + offset;
  debug_save("Seeking to " + std::to_string(_pos) + " in savefile.");
  if (_fatFile != nullptr) {
    fseek(_fatFile, _pos, SEEK_SET);
  }
}

int CardBuffer::tell() const {
  if (!_opened)
    return -1;
  return _pos;
}

CardBuffer fCard;
