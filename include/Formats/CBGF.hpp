// Cervi BackGround Format
#ifndef UNDERTALE_CBGF_HPP
#define UNDERTALE_CBGF_HPP

#include <nds.h>

struct CBGFHeader {
    char header[4] = {'C', 'B', 'G', 'F'};
    u32 fileSize = 0;

    static constexpr u32 version = 2;

    // abcd efgh
    // abcdefg - unused for now
    // h - bool (if color depth == 8)
    u8 fileFormat = 0;
};

struct CBGFPalette {
    u8 colorCount = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    u16 *colorData = nullptr;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CBGFTiles {
    u16 tileCount = 0;  // 0 - 1024
    u8* tileData = nullptr; // 8x8, if color_depth==4 each pixel is 4 bits (32B) else 8 bits (64B) (index into palette + 1)
};

struct CBGFMap {
    u16 width = 0, height = 0;  // In pixels
    u16* tileMap = nullptr;  // 0 - 1023 (index cbgf tiles)
};

#endif