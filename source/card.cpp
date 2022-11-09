//
// Created by cervi on 28/09/2022.
//

#include "card.hpp"

u8 cardCommand(u8 command, bool hold) {
    REG_AUXSPICNT = 0xa000 + (hold ? 0x40 : 0);
    eepromWaitBusy();
    REG_AUXSPIDATA = command;
    eepromWaitBusy();
    return REG_AUXSPIDATA;
}

u8 cardTransfer(u8 data) {
    REG_AUXSPIDATA = data;
    eepromWaitBusy();
    return REG_AUXSPIDATA;
}

void cardWaitInProgress() {
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
    cardReadBytes((u8*)data, _pos, size);
    _pos += size;
}

void CardBuffer::write(void *src, size_t size) {
    cardWriteBytes((u8*)src, _pos, size);
    _pos += size;
}

void CardBuffer::seek(s32 offset, u8 mode) {
    if (mode == SEEK_SET)
        _pos = offset;
    else if (mode == SEEK_CUR)
        _pos += offset;
    else if (mode == SEEK_END)
        _pos = 7999 + offset;
}

CardBuffer fCard;
