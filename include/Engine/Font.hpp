//
// Created by cervi on 23/08/2022.
//

#ifndef LAYTON_FONT_HPP
#define LAYTON_FONT_HPP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ARM9
#include <nds.h>
#include "Formats/CFNT.hpp"

namespace Engine {
    class Font {
    public:
        int loadCFNT(FILE* f);
        bool getLoaded() const { return loaded; }
        CFNTGlyph* getGlyph(int glyphIdx) const { return &glyphs.glyphs[glyphIdx - 1]; }
        uint8_t* getGlyphMap() { return glyphMap.glyphMap; }
        void free_();
        ~Font() { free_(); }
    private:
        bool loaded = false;
        CFNTGlyphs glyphs;
        CFNTMap glyphMap;
    };

    class TextBGManager {
    public:
        TextBGManager(u16* paletteRam, u16* tileRam, u16* mapRam) :
                      paletteRam(paletteRam), tileRam(tileRam), mapRam(mapRam) {
            paletteRam[16 * 15 + 0] = 31 << 5;  // full green color (transparent)
            paletteRam[16 * 15 + 11] = 0;  // black color
            paletteRam[16 * 15 + 12] = 31;  // full red color
            paletteRam[16 * 15 + 13] = 31 << 5;  // full green color
            paletteRam[16 * 15 + 14] = 31 << 10;  // full blue color
            paletteRam[16 * 15 + 15] = (31 << 10) + (31 << 5) + 31;  // full white color
        }
        void drawGlyph(Font& font, uint8_t glyph, int &x, int y);
        void setPaletteColor(int colorIdx, int r, int g, int b, bool color8bit);
        void setCurrentColor(int colorIdx) { paletteColor = colorIdx; }
        void clear();
    private:
        uint8_t* getTile(int x, int y);

        u16* paletteRam;
        u16* tileRam;
        u16* mapRam;
        int tileReserve = 1;
        int paletteColor = 15;
    };

    extern TextBGManager textMain;
    extern TextBGManager textSub;
}

#endif //LAYTON_FONT_HPP
