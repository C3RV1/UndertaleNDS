#ifndef UNDERTALE_CSPR_HPP
#define UNDERTALE_CSPR_HPP

#define ARM9
#include <nds.h>

struct CSPRHeader {
    char header[4] = {'C', 'S', 'P', 'R'};
    u32 fileSize = 0;

    u32 version = 5;

    u16 width = 0;
    u16 height = 0;
    u16 topDownOffset = 0;
};

struct CSPRColors {
    u8 count = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    u16 *colorData = nullptr;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CSPRTiles {
    u8 frameCount = 0;
    u8* tileData = nullptr;
};

struct CSPRAnimFrame {
    u8 frame = 0;
    u16 duration = 0;
    s8 drawOffX = 0;
    s8 drawOffY = 0;
};

struct CSPRAnimation {
    char* name = nullptr;
    u8 frameCount = 0;
    CSPRAnimFrame* frames = nullptr;
};

struct CSPRAnimations {
    u8 count = 0;
    CSPRAnimation* animations = nullptr;
};

#endif //UNDERTALE_CSPR_HPP
