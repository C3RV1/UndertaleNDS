//
// Created by cervi on 23/08/2022.
//

#include "Engine/Font.hpp"

namespace Engine {
    int Font::loadCFNT(FILE *f) {
        free_();
        char header[4];
        uint32_t fileSize;
        uint32_t version;
        fread(header, 4, 1, f);

        const char expectedChar[4] = {'C', 'F', 'N', 'T'};
        if (memcmp(header, expectedChar, 4) != 0) {
            return 1;
        }

        fread(&fileSize, 4, 1, f);
        uint32_t pos = ftell(f);
        fseek(f, 0, SEEK_END);
        uint32_t size = ftell(f);
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
        glyphs.glyphs = (CFNTGlyph*) malloc(sizeof(CFNTGlyph) * glyphs.glyphCount);

        for (int i = 0; i < glyphs.glyphCount; i++) {
            CFNTGlyph* glyph = &glyphs.glyphs[i];
            fread(&glyph->width, 1, 1, f);
            fread(&glyph->height, 1, 1, f);
            fread(&glyph->shift, 1, 1, f);
            fread(&glyph->offset, 1, 1, f);
            uint16_t dataBytes = ((glyph->width * glyph->height + 7) / 8);
            glyph->glyphData = (uint8_t*) malloc(dataBytes);
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
            free(glyphs.glyphs[glyphIdx].glyphData);
        }
        free(glyphs.glyphs);
    }

    void TextBGManager::drawGlyph(Font& font, uint8_t glyph, int &x, int y) {
        if (!font.getLoaded())
            return;

        uint8_t glyphIdx = font.getGlyphMap()[glyph];
        if (glyphIdx == 0)
            return;
        CFNTGlyph* glyphObj = font.getGlyph(glyphIdx);
        int endX = x + glyphObj->shift;
        x += glyphObj->offset;
        for (uint8_t glyphY = 0;glyphY < glyphObj->height && y < 192;) {
            int x_ = x;
            for (uint8_t glyphX = 0;glyphX < glyphObj->width && x_ < 256;) {
                uint8_t* tilePointer = getTile(x_, y);
                uint8_t tileX = x_ % 8;
                uint8_t tileY = y % 8;
                uint8_t* tileByte = tilePointer + (((tileY * 8 + tileX) / 2) & (~1));
                auto* tile = (uint16_t*) tileByte;

                bool highBits = (tileX & 1) == 1;
                bool prevByte = (((tileY * 8 + tileX) / 2) & 1) == 1;

                uint32_t bitPos = glyphY * glyphObj->width + glyphX;
                uint32_t byte = bitPos / 8;
                bitPos = 7 - (bitPos % 8);

                uint8_t bit = glyphObj->glyphData[byte] >> bitPos;

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

    void TextBGManager::clear() {
        memset(mapRam, 0, 2 * 32 * 32);
        tileReserve = 1;
    }

    uint8_t* TextBGManager::getTile(int x, int y) {
        x /= 8;
        y /= 8;
        uint16_t tileId = *((uint8_t *) mapRam + (y * 32 + x) * 2);
        if (tileId == 0) {
            tileId = tileReserve++;
            *(uint16_t*)((uint8_t *) mapRam + (y * 32 + x) * 2) = (15 << 12) + tileId;
            memset(((uint8_t*)tileRam) + (tileId * 32), 0, 32); // Initialize tile to blank
        }
        return ((uint8_t*)tileRam) + (tileId * 32);
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
