//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_CFNT_HPP
#define UNDERTALE_CFNT_HPP

#include <stdint.h>

struct CFNTHeader {
    char header[4] = {'C', 'F', 'N', 'T'};
    u32 fileSize = 0;
    u32 version = 1;
};

struct CFNTGlyph {
    u8 width;
    u8 height;
    u8 shift; // How many pixels moves the current x forward
    u8 offset;  // How much x offset when rendering
    u8* glyphData;  // width x height (rows first), 1 bit per pixel, rounded to byte
};

struct CFNTGlyphs {
    u8 lineHeight;
    u8 glyphCount; // (0-255) glyph id 0 reserved for not defined
    CFNTGlyph* glyphs;
};

struct CFNTMap {
    u8 glyphMap[256];  // 0 == not defined, else glyphIdx + 1
};

#endif //UNDERTALE_CFNT_HPP
