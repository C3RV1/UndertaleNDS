// Cervi BackGround Format
#ifndef UNDERTALE_CBGF_HPP
#define UNDERTALE_CBGF_HPP

#include <stdint.h>
#include <stdio.h>

struct CBGFHeader {
    char header[4] = {'C', 'B', 'G', 'F'};
    uint32_t fileSize = 0;

    uint32_t version = 1;

    // abcd efgh
    // abcdefg - unused for now
    // h - bool (if color depth == 8)
    uint8_t fileFormat = 0;
};

struct CBGFPalette {
    uint8_t colorCount = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    uint16_t *colorData = nullptr;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CBGFTiles {
    uint16_t tileCount = 0;  // 0 - 1024
    uint8_t* tileData = nullptr; // 8x8, if color_depth==4 each pixel is 4 bits (32B) else 8 bits (64B) (index into palette + 1)
};

struct CBGFMap {
    uint16_t width = 0, height = 0;
    uint16_t* tileMap = nullptr;  // 0 - 1023 (index cbgf tiles)
};

#endif