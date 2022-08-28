#ifndef LAYTON_CSPR_HPP
#define LAYTON_CSPR_HPP

#include <stdint.h>

struct CSPRHeader {
    char header[4] = {'C', 'S', 'P', 'R'};
    uint32_t fileSize = 0;

    uint32_t version = 2;

    uint8_t tileWidth = 0;
    uint8_t tileHeight = 0;
};

struct CSPRColors {
    uint8_t count = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    uint16_t *colorData = nullptr;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CSPRTiles {
    uint8_t frameCount;
    uint8_t* tileData;
};

struct CSPRAnimFrame {
    uint8_t frame;
    uint16_t duration;
};

struct CSPRAnimation {
    char* name;
    uint8_t frameCount;
    CSPRAnimFrame* frames;
};

struct CSPRAnimations {
    uint8_t count;
    CSPRAnimation* animations;
};

#endif //LAYTON_CSPR_HPP
