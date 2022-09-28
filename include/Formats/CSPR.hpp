#ifndef UNDERTALE_CSPR_HPP
#define UNDERTALE_CSPR_HPP

#include <stdint.h>

struct CSPRHeader {
    char header[4] = {'C', 'S', 'P', 'R'};
    u32 fileSize = 0;

    u32 version = 4;

    u16 width = 0;
    u16 height = 0;
};

struct CSPRColors {
    u8 count = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    u16 *colorData = nullptr;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CSPRTiles {
    u8 frameCount;
    u8* tileData;
};

struct CSPRAnimFrame {
    u8 frame;
    u16 duration;
    s8 drawOffX;
    s8 drawOffY;
};

struct CSPRAnimation {
    char* name;
    u8 frameCount;
    CSPRAnimFrame* frames;
};

struct CSPRAnimations {
    u8 count;
    CSPRAnimation* animations;
};

#endif //UNDERTALE_CSPR_HPP
