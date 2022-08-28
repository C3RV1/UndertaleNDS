//
// Created by cervi on 23/08/2022.
//

#ifndef LAYTON_CFNT_HPP
#define LAYTON_CFNT_HPP

#include <stdint.h>

struct CFNTHeader {
    char header[4] = {'C', 'F', 'N', 'T'};
    uint32_t fileSize = 0;
    uint32_t version = 1;
};

struct CFNTGlyph {
    uint8_t width;
    uint8_t height;
    uint8_t shift; // How many pixels moves the current x forward
    uint8_t offset;  // How much x offset when rendering
    uint8_t* glyphData;  // width x height (rows first), 1 bit per pixel, rounded to byte
};

struct CFNTGlyphs {
    uint8_t lineHeight;
    uint8_t glyphCount; // (0-255) glyph id 0 reserved for not defined
    CFNTGlyph* glyphs;
};

struct CFNTMap {
    uint8_t glyphMap[256];  // 0 == not defined, else glyphIdx + 1
};

#endif //LAYTON_CFNT_HPP
