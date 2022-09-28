//
// Created by cervi on 23/08/2022.
//

#include "Engine/Font.hpp"

namespace Engine {
    int Font::loadCFNT(FILE *f) {
        free_();
        char header[4];
        u32 fileSize;
        u32 version;
        fread(header, 4, 1, f);

        const char expectedChar[4] = {'C', 'F', 'N', 'T'};
        if (memcmp(header, expectedChar, 4) != 0) {
            return 1;
        }

        fread(&fileSize, 4, 1, f);
        u32 pos = ftell(f);
        fseek(f, 0, SEEK_END);
        u32 size = ftell(f);
        fseek(f, pos, SEEK_SET);

        if (fileSize != size) {
            return 2;
        }

        fread(&version, 4, 1, f);
        if (version != 1) {
            return 3;
        }

        fread(&glyphs.lineHeight, 1, 1, f);
        fread(&glyphs.glyphCount, 1, 1, f);
        glyphs.glyphs = new CFNTGlyph[glyphs.glyphCount];

        for (int i = 0; i < glyphs.glyphCount; i++) {
            CFNTGlyph* glyph = &glyphs.glyphs[i];
            fread(&glyph->width, 1, 1, f);
            fread(&glyph->height, 1, 1, f);
            fread(&glyph->shift, 1, 1, f);
            fread(&glyph->offset, 1, 1, f);
            u16 dataBytes = ((glyph->width * glyph->height + 7) / 8);
            glyph->glyphData = new u8[dataBytes];
            fread(glyph->glyphData, dataBytes, 1, f);
        }

        fread(glyphMap.glyphMap, 1, 256, f);

        loaded = true;
        return 0;
    }

    void Font::free_() {
        if (!loaded)
            return;
        loaded = false;

        for (int glyphIdx = 0; glyphIdx < glyphs.glyphCount; glyphIdx++) {
            delete[] glyphs.glyphs[glyphIdx].glyphData;
            glyphs.glyphs[glyphIdx].glyphData = nullptr;
        }
        delete[] glyphs.glyphs;
        glyphs.glyphs = nullptr;
    }

    void TextBGManager::drawGlyph(Font& font, u8 glyph, int &x, int y) {
        if (!font.getLoaded())
            return;

        u8 glyphIdx = font.getGlyphMap()[glyph];
        if (glyphIdx == 0)
            return;
        CFNTGlyph* glyphObj = font.getGlyph(glyphIdx);
        int endX = x + glyphObj->shift;
        x += glyphObj->offset;
        for (u8 glyphY = 0;glyphY < glyphObj->height && y < 192;) {
            int x_ = x;
            for (u8 glyphX = 0;glyphX < glyphObj->width && x_ < 256;) {
                u8* tilePointer = getTile(x_, y);
                u8 tileX = x_ % 8;
                u8 tileY = y % 8;
                u8* tileByte = tilePointer + (((tileY * 8 + tileX) / 2) & (~1));
                auto* tile = (u16*) tileByte;

                bool highBits = (tileX & 1) == 1;
                bool prevByte = (((tileY * 8 + tileX) / 2) & 1) == 1;

                u32 bitPos = glyphY * glyphObj->width + glyphX;
                u32 byte = bitPos / 8;
                bitPos = 7 - (bitPos % 8);

                u8 bit = glyphObj->glyphData[byte] >> bitPos;

                if (bit & 1) {
                    *tile &= ~(0xF << (4 * highBits) << (8 * prevByte));
                    *tile += paletteColor << (4 * highBits) << (8 * prevByte);
                }

                glyphX++;
                x_++;
            }
            glyphY++;
            y++;
        }
        x = endX;
    }

    u8 TextBGManager::getGlyphWidth(Font& font, u8 glyph) {
        if (!font.getLoaded())
            return 0;

        u8 glyphIdx = font.getGlyphMap()[glyph];
        if (glyphIdx == 0)
            return 0;
        CFNTGlyph* glyphObj = font.getGlyph(glyphIdx);
        return glyphObj->shift;
    }

    void TextBGManager::reloadColors() {
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

    void TextBGManager::clear() {
        memset(mapRam, 0, 2 * 32 * 32);
        tileReserve = 1;
    }

    u8* TextBGManager::getTile(int x, int y) {
        x /= 8;
        y /= 8;
        u16 tileId = *((u8 *) mapRam + (y * 32 + x) * 2);
        if (tileId == 0) {
            tileId = tileReserve++;
            *(u16*)((u8 *) mapRam + (y * 32 + x) * 2) = (15 << 12) + tileId;
            memset(((u8*)tileRam) + (tileId * 32), 0, 32); // Initialize tile to blank
        }
        return ((u8*)tileRam) + (tileId * 32);
    }

    void TextBGManager::setPaletteColor256(int colorIdx, int r, int g, int b, bool color8bit) {
        if (color8bit) {
            r >>= 3;
            g >>= 3;
            b >>= 3;
        }
        paletteRam[16 * 15 + colorIdx] = (b << 10) + (g << 5) + r;
    }

    TextBGManager textMain(BG_PALETTE, BG_TILE_RAM(5), BG_MAP_RAM(4));
    TextBGManager textSub(BG_PALETTE_SUB, BG_TILE_RAM_SUB(5), BG_MAP_RAM_SUB(1));
}
