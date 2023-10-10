//
// Created by cervi on 28/09/2022.
//

#include <cstdio>
#include "card.hpp"
#include "Engine/Engine.hpp"
#include <fat.h>

void CardBuffer::open(const char *mode) {
    if (_opened)
        return;
    _pos = 0;
    _running_in_fat = access("sd:/", F_OK) == 0; // Returns 0 on success
    if (_running_in_fat)  // check if sd was inited
        _fatFile = fopen("sd:/Undertale.save", mode);
    else {
        _fatFile = nullptr;
    }
    _opened = true;
}

void CardBuffer::close() {
    if (!_opened)
        return;
    if (_fatFile != nullptr) {
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

void cardReadBytes(u8* dst, u32 addr, u16 size) {
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
}

void cardWriteBytes(u8* src, u32 addr, u16 size) {
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
}

void CardBuffer::read(void *data, size_t size) {
    if (!_opened)
        return;
    if (!_running_in_fat) {
        cardReadBytes((u8*)data, _pos, size);
    }
    else if (_fatFile != nullptr) {
        size_t bytes_read = fread(data, size, 1, _fatFile);
        for (;bytes_read < size; bytes_read++) {
            static_cast<u8*>(data)[bytes_read] = 0xff;
        }
    }
    else {
        // We are running in fat but file was not opened
        // We simulate that behaviour by writing 0xff
        for (size_t bytes_read = 0; bytes_read < size; bytes_read++) {
            static_cast<u8*>(data)[bytes_read] = 0xff;
        }
    }
    _pos += size;
}

void CardBuffer::write(void *src, size_t size) {
    if (!_opened)
        return;
    if (_running_in_fat) {
        cardWriteBytes((u8*)src, _pos, size);
    }
    else if (_fatFile != nullptr) {
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
