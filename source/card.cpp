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

void CardBuffer::open(const char *mode) {
  if (_opened)
    return;
  _pos = 0;
  _opened = true;

  _running_in_file = access("sd:/", F_OK) == 0; // Returns 0 on success
  if (_running_in_file) {                       // check if sd was inited
#ifdef DEBUG_SAVE
    nocashMessage("Running in sd.");
#endif

    _fatFile = fopen("sd:/Undertale.sav", mode);

#ifdef DEBUG_SAVE
    if (_fatFile)
      nocashMessage("SaveFile opened.");
    else {
      std::string buf =
          "SaveFile couldn't be opened. Errno: " + std::to_string(errno);
      nocashMessage(buf.c_str());
    }
#endif

    return;
  }

  _running_in_file = access("fat:/", F_OK) == 0; // Returns 0 on success
  if (_running_in_file) {                        // check if dldi was inited
#ifdef DEBUG_SAVE
    nocashMessage("Running in fat.");
#endif

    _fatFile = fopen("fat:/Undertale.sav", mode);

#ifdef DEBUG_SAVE
    if (_fatFile)
      nocashMessage("SaveFile opened.");
    else {
      std::string buf =
          "SaveFile couldn't be opened. Errno: " + std::to_string(errno);
      nocashMessage(buf.c_str());
    }
#endif

    return;
  }

#ifdef DEBUG_SAVE
  nocashMessage("Not running in fat.");
#endif
  _fatFile = nullptr;
}

void CardBuffer::close() {
  if (!_opened)
    return;
#ifdef DEBUG_SAVE
  nocashMessage("Closing card buffer.");
#endif
  if (_fatFile != nullptr) {
#ifdef DEBUG_SAVE
    nocashMessage("Closing savefile buffer.");
#endif
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
  cardCommand(0, false);

  if (REG_AUXSPIDATA == 0xFF)
    Engine::throw_("Error accessing savefile.");

  do {
    cardCommand(SPI_EEPROM_RDSR, true);
    cardCommand(0, false);
  } while (REG_AUXSPIDATA & 1);
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

void CardBuffer::read(void *data, size_t size) {
  if (!_opened)
    return;
  if (!_running_in_file) {
#ifdef DEBUG_SAVE
    nocashMessage("Reading from card.");
#endif
    cardReadBytes((u8 *)data, _pos, size);
  } else if (_fatFile != nullptr) {
    size_t bytes_read = fread(data, 1, size, _fatFile);
#ifdef DEBUG_SAVE
    std::string buf = "Read " + std::to_string(bytes_read) + " bytes of " +
                      std::to_string(size);
    nocashMessage(buf.c_str());
#endif
    for (; bytes_read < size; bytes_read++) {
      static_cast<u8 *>(data)[bytes_read] = 0xff;
    }
  } else {
#ifdef DEBUG_SAVE
    nocashMessage("Reading from unopened file.");
#endif
    // We are running in fat but file was not opened
    // We simulate that behaviour by writing 0xff
    for (size_t bytes_read = 0; bytes_read < size; bytes_read++) {
      static_cast<u8 *>(data)[bytes_read] = 0xff;
    }
  }
  _pos += size;
}

void CardBuffer::write(void *src, size_t size) {
  if (!_opened)
    return;
  if (!_running_in_file) {
#ifdef DEBUG_SAVE
    std::string buf = "Writing " + std::to_string(size) + " bytes to card.";
    nocashMessage(buf.c_str());
#endif
    cardWriteBytes((u8 *)src, _pos, size);
  } else if (_fatFile != nullptr) {
#ifdef DEBUG_SAVE
    std::string buf = "Writing " + std::to_string(size) + " bytes to savefile.";
    nocashMessage(buf.c_str());
#endif
    fwrite(src, size, 1, _fatFile);
  }
  // The else branch would be running in file but file couldn't be opened
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
#ifdef DEBUG_SAVE
  std::string buf = "Seeking to " + std::to_string(_pos) + " in savefile.";
  nocashMessage(buf.c_str());
#endif
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
