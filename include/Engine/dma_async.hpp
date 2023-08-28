//
// Created by cervi on 25/08/2023.
//

#ifndef UNDERTALE_DMA_ASYNC_HPP
#define UNDERTALE_DMA_ASYNC_HPP


#include <nds.h>

// These are the same as libnds, but check that the channel
// is available before using it, and provides the
// fill words/halfwords async variants

extern "C" {
static inline void dmaCopyWordsAsync(uint8 channel, const void *src, void *dest, uint32 size) {
    while (DMA_CR(channel) & DMA_BUSY);
    DMA_SRC(channel) = (uint32) src;
    DMA_DEST(channel) = (uint32) dest;
    DMA_CR(channel) = DMA_COPY_WORDS | (size >> 2);

}

static inline void dmaCopyHalfWordsAsync(uint8 channel, const void *src, void *dest, uint32 size) {
    while (DMA_CR(channel) & DMA_BUSY);
    DMA_SRC(channel) = (uint32) src;
    DMA_DEST(channel) = (uint32) dest;
    DMA_CR(channel) = DMA_COPY_HALFWORDS | (size >> 1);
}

static inline void dmaFillWordsAsync(uint8 channel, u32 value, void *dest, uint32 size) {
    while (DMA_CR(channel) & DMA_BUSY);
#ifdef ARM7
    (*(vu32*)0x027FFE04) = value;
    DMA_SRC(channel) = 0x027FFE04;
#else
    DMA_FILL(channel) = value;
    DMA_SRC(channel) = (uint32) &DMA_FILL(channel);
#endif

    DMA_DEST(channel) = (uint32) dest;
    DMA_CR(channel) = DMA_SRC_FIX | DMA_COPY_WORDS | (size >> 2);
}

static inline void dmaFillHalfWordsAsync(uint8 channel, u16 value, void *dest, uint32 size) {
    while (DMA_CR(channel) & DMA_BUSY);
#ifdef ARM7
    (*(vu32*)0x027FFE04) = (u32)value;
    DMA_SRC(channel) = 0x027FFE04;
#else
    DMA_FILL(channel) = (uint32) value;
    DMA_SRC(channel) = (uint32) &DMA_FILL(channel);
#endif

    DMA_DEST(channel) = (uint32) dest;
    DMA_CR(channel) = DMA_SRC_FIX | DMA_COPY_HALFWORDS | (size >> 1);
}
}

#endif //UNDERTALE_DMA_ASYNC_HPP
