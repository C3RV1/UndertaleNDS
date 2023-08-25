#ifndef UNDERTALE_CSPR_HPP
#define UNDERTALE_CSPR_HPP

#define ARM9
#include <nds.h>
#include <string>
#include <memory>

struct CSPRHeader {
    char header[4] = {'C', 'S', 'P', 'R'};
    u32 fileSize = 0;

    u32 version = 6;

    u16 width = 0;
    u16 height = 0;
    u16 topDownOffset = 0;
    u16 frameCount = 0;

    bool containsOam = true;
    bool contains3d = true;
};

struct CSPRColors {
    u8 count = 0;  // 0 - 249 (color 0 is reserved, and colors 250-255 too for text)
    std::vector<u16> colorData;  // 2 bytes per color (bits 0-4 r, bits 5-9 g, bits 10-15 b, bit 16 unused)
};

struct CSPRAnimFrame {
    u8 frame = 0;
    u16 duration = 0;
    s8 drawOffX = 0;
    s8 drawOffY = 0;
};

struct CSPRAnimation {
    std::string name;
    std::vector<CSPRAnimFrame> frames;
};

struct CSPRAnimations {
    u8 count = 0;
    std::vector<CSPRAnimation> animations;
};

struct CSPR_3D_Tile {
    u8 tileWidth = 0;
    u8 tileHeight = 0;
    std::vector<u8> tileFrameData;
};

struct CSPR_3D {
    u8 tilesAllocX = 0;
    u8 tilesAllocY = 0;
    std::vector<CSPR_3D_Tile> tiles;
};

struct CSPR_OAM_Entry {
    u8 tilesW = 0;
    u8 tilesH = 0;
    std::vector<u8> tilesFrameData;
};

struct CSPR_OAM {
    u8 oamW = 0;
    u8 oamH = 0;
    std::vector<CSPR_OAM_Entry> oamEntries;
};

struct CSPRFile {
    CSPRHeader header;
    CSPRColors colors;
    CSPRAnimation animations;
    CSPR_OAM oam; // Optional
    CSPR_3D _3d; // Optional
};

#endif //UNDERTALE_CSPR_HPP
