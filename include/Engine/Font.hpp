//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_FONT_HPP
#define UNDERTALE_FONT_HPP

#include <cstdio>
#include <string>

#include <nds.h>
#include "Formats/CFNT.hpp"

namespace Engine {
    class Font {
    public:
        bool loadPath(const std::string& path);
        void loadCFNT(FILE* f);
        bool getLoaded() const { return _loaded; }
        u8 getGlyphWidth(u8 glyph);
        ~Font() { free_(); }
    private:
        u8* getGlyphMap() { return _glyphMap.glyphMap; }
        const CFNTGlyph *getGlyph(int glyphIdx) const { return &_glyphs.glyphs[glyphIdx - 1]; }
        friend class TextBGManager;
        bool _loaded = false;
        CFNTGlyphs _glyphs;
        CFNTMap _glyphMap;
        void free_();

        std::string _path;
    };

    class TextBGManager {
    public:
        TextBGManager(u16* paletteRam, u16* tileRam, u16* mapRam) :
                _paletteRam(paletteRam), _tileRam(tileRam), _mapRam(mapRam) {
            paletteRam[16 * 15 + 0] = 31 << 5;  // full green color (transparent)
            paletteRam[16 * 15 + 8] = 0;  // black color
            paletteRam[16 * 15 + 9] = 31;  // full red color
            paletteRam[16 * 15 + 10] = 31 << 5;  // full green color
            paletteRam[16 * 15 + 11] = 31 << 10;  // full blue color
            paletteRam[16 * 15 + 12] = 31 + (31 << 5);  // red + green = yellow
            paletteRam[16 * 15 + 13] = 31 + (31 << 10);  // red + blue = purple
            paletteRam[16 * 15 + 14] = (31 << 5) + (31 << 10);  // green + blue = turquoise
            paletteRam[16 * 15 + 15] = (31 << 10) + (31 << 5) + 31;  // full white color
        }
        void drawGlyph(Font& font, u8 glyph, int &x, int y);
        void reloadColors();
        void setPaletteColor(int colorIdx, int r, int g, int b, bool color8bit);
        void setPaletteColor(int colorIdx, u16 color5bit);
        void setColor(int colorIdx) { _paletteColor = colorIdx; }
        u16 getColor() const { return _paletteColor; }
        void clear();
        void clearRect(int x, int y, int w, int h);
    private:
        u8* getTile(int x, int y);

        u16* _paletteRam;
        u16* _tileRam;
        u16* _mapRam;
        int _tileReserve = 1;
        int _paletteColor = 15;
    };

    extern TextBGManager textMain;
    extern TextBGManager textSub;
}

#endif //UNDERTALE_FONT_HPP
