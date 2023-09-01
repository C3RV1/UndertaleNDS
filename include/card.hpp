//
// Created by cervi on 28/09/2022.
//

#ifndef UNDERTALE_CARD_HPP
#define UNDERTALE_CARD_HPP

#include <nds.h>

u8 cardCommand(u8 command, bool hold);
u8 cardTransfer(u8 data);
void cardWaitInProgress();
void cardReadBytes(u8* dst, u32 addr, u16 size);
void cardWriteBytes(u8* src, u32 addr, u16 size);

class CardBuffer {
public:
    void open(const char* mode);
    void close();
    void read(void* data, size_t size);
    void write(void* src, size_t size);
    int tell() const;
    void seek(s32 offset, u8 mode);
    ~CardBuffer() {close();}
private:
    int _pos = 0;
    FILE* _fatFile = nullptr;
    bool _opened = false;
};

extern CardBuffer fCard;

#endif //UNDERTALE_CARD_HPP
