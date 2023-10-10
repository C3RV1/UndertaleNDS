//
// Created by cervi on 08/10/2023.
//
#include <cstdint>
#include <cstdlib>
#include "Engine/dma.hpp"

// Based on NFLib and http://www.coranac.com/2009/05/dma-vs-arm9-fight/
void dmaCopySafe(u8 channel, const void *src, void* dst, size_t size) {
    if (size == 0)
        return;

    auto source = reinterpret_cast<std::uintptr_t>(src);
    auto destination = reinterpret_cast<std::uintptr_t>(dst);

    if ((source | destination) & 1) {
        memcpy(dst, src, size);
        return;
    }

    while (dmaBusy(channel));

    DC_FlushRange(src, size);

    if ((source | destination | size) & 3)
        dmaCopyHalfWords(channel, src, dst, size);
    else
        dmaCopyWords(channel, src, dst, size);

    DC_InvalidateRange(dst, size);
}

void dmaFillSafe(u8 channel, u32 value, void* dst, size_t size) {
    if (size == 0)
        return;

    auto destination = reinterpret_cast<std::uintptr_t>(dst);

    if (destination & 1) {
        memset(dst, value, size);
        return;
    }

    while (dmaBusy(channel));

    if ((destination | size) & 3)
        dmaFillHalfWordsAsync(channel, value & 0xFFFF, dst, size);
    else
        dmaFillWordsAsync(channel, value, dst, size);

    DC_InvalidateRange(dst, size);
}