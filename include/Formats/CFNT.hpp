//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_CFNT_HPP
#define UNDERTALE_CFNT_HPP

#include <nds.h>
#include <vector>

struct CFNTHeader {
    char header[4] = {'C', 'F', 'N', 'T'};
    u32 fileSize = 0;
    static constexpr u32 version = 1;
};

struct CFNTGlyph {
    u8 width = 0;
    u8 height = 0;
    u8 shift = 0; // How many pixels moves the current x forward
    u8 offset = 0;  // How much x offset when rendering
    std::vector<u8> glyphData;  // width x height (rows first), 1 bit per pixel, rounded to byte
};

struct CFNTGlyphs {
    u8 lineHeight = 0;
    std::vector<CFNTGlyph> glyphs;
};

struct CFNTMap {
    u8 glyphMap[256] = {0};  // 0 == not defined, else glyphIdx + 1
};

#endif //UNDERTALE_CFNT_HPP
