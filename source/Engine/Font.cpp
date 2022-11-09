//
// Created by cervi on 23/08/2022.
//

#include "Engine/Font.hpp"

namespace Engine {
    bool Font::loadPath(const char *path) {
        char pathFull[100];
        char buffer[100];

        sprintf(pathFull, "nitro:/fnt/%s.cfnt", path);

        FILE* f = fopen(pathFull, "rb");
        if (!f) {
            sprintf(buffer, "Error opening fnt %s", path);
            nocashMessage(buffer);
            return false;
        }

        int loadRes = loadCFNT(f);

        fclose(f);

        if (loadRes != 0) {
            sprintf(buffer, "Error loading fnt %s: %d", path, loadRes);
            nocashMessage(buffer);
            return false;
        }

        return true;
    }

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

        fread(&_glyphs.lineHeight, 1, 1, f);
        fread(&_glyphs.glyphCount, 1, 1, f);
        _glyphs.glyphs = new CFNTGlyph[_glyphs.glyphCount];

        for (int i = 0; i < _glyphs.glyphCount; i++) {
            CFNTGlyph* glyph = &_glyphs.glyphs[i];
            fread(&glyph->width, 1, 1, f);
            fread(&glyph->height, 1, 1, f);
            fread(&glyph->shift, 1, 1, f);
            fread(&glyph->offset, 1, 1, f);
            u16 dataBytes = ((glyph->width * glyph->height + 7) / 8);
            glyph->glyphData = new u8[dataBytes];
            fread(glyph->glyphData, dataBytes, 1, f);
        }

        fread(_glyphMap.glyphMap, 1, 256, f);

        _loaded = true;
        return 0;
    }

    void Font::free_() {
        if (!_loaded)
            return;
        _loaded = false;

        for (int glyphIdx = 0; glyphIdx < _glyphs.glyphCount; glyphIdx++) {
            delete[] _glyphs.glyphs[glyphIdx].glyphData;
            _glyphs.glyphs[glyphIdx].glyphData = nullptr;
        }
        delete[] _glyphs.glyphs;
        _glyphs.glyphs = nullptr;
    }

    void TextBGManager::drawGlyph(Font& font, u8 glyph, int &x, int y) {
        if (!font._loaded)
            return;

        u8 glyphIdx = font._glyphMap.glyphMap[glyph];
        if (glyphIdx == 0)
            return;
        CFNTGlyph* glyphObj = font.getGlyph(glyphIdx);
        int endX = x + glyphObj->shift;
        x += glyphObj->offset;

        // Until we haven't completed the glyph in the y-axis
        for (u8 glyphY = 0;glyphY < glyphObj->height && y < 192;) {

            // Copy x, when we go back to the start of the line we want to keep the starting x
            int x_ = x;

            // Until we haven't completed the glyph in the x-axis
            // in the current vertical strip (tiles are 8x8, vertical strip are all
            //                                tiles on the same row)
            for (u8 glyphX = 0;glyphX < glyphObj->width && x_ < 256;) {
                // Pointer to the current tile
                u8* tilePointer = getTile(x_, y);

                // Offset in current tile
                u8 tileX = x_ % 8;
                u8 tileY = y % 8;

                // Copy the current glyph y position
                int glyphY_ = glyphY;

                // Until we haven't completed the glyph in the current tile vertically
                for (;tileY < 8 && y / 8 + tileY < 192 && glyphY_ < glyphObj->height; tileY++) {

                    // Copy tile x and glyph x
                    // When we go back to the start of the tile we want to keep these
                    int tileX_ = tileX;
                    int glyphX_ = glyphX;

                    // Until we haven't completed the glyph in the current tile horizontally
                    for (;tileX_ < 8 && x_ / 8 + tileX_ < 256 && glyphX_ < glyphObj->width; tileX_++) {
                        // Get pointer to the current pixel (can only write in words, so we round to
                        // the word (& (~1)). Then we'll shift the bits accordingly).
                        u8* tileByte = tilePointer + (((tileY * 8 + tileX_) / 2) & (~1));
                        auto* tile = (u16*) tileByte;

                        // Are we writing to the high bits of the byte (0xX0) or low bits (0x0X)?
                        bool highBits = (tileX_ & 1) == 1;

                        // Are we writing to the byte coming before (0xXX00) or memory of after (0x00XX)
                        // (because we can only write in words)?
                        bool prevByte = (((tileY * 8 + tileX_) / 2) & 1) == 1;

                        // Position of glyph bit (glyphs are 1 bit depth)
                        u32 bitPos = glyphY_ * glyphObj->width + glyphX_;
                        u32 byte = bitPos / 8;
                        bitPos = 7 - (bitPos % 8);

                        // Get the pixel of the glyph as the lowest significant bit (0b0000000x)
                        u8 bit = glyphObj->glyphData[byte] >> bitPos;

                        if (bit & 1) {
                            // Clear tile position and write palette color
                            *tile &= ~(0xF << (4 * highBits) << (8 * prevByte));
                            *tile += _paletteColor << (4 * highBits) << (8 * prevByte);
                        }
                        glyphX_++;
                    }
                    glyphY_++;
                }

                // Move to next tile in row
                glyphX += 8 - (x_ % 8);
                x_ += 8 - (x_ % 8);
            }

            // Move to next vertical strip
            glyphY += 8 - (y % 8);
            y += 8 - (y % 8);
        }
        x = endX;
    }

    u8 Font::getGlyphWidth(u8 glyph) {
        u8 glyphIdx = getGlyphMap()[glyph];
        if (glyphIdx == 0)
            return 0;
        CFNTGlyph* glyphObj = getGlyph(glyphIdx);
        return glyphObj->shift;
    }

    void TextBGManager::reloadColors() {
        _paletteRam[16 * 15 + 0] = 31 << 5;  // full green color (transparent)
        _paletteRam[16 * 15 + 8] = 0;  // black color
        _paletteRam[16 * 15 + 9] = 31;  // full red color
        _paletteRam[16 * 15 + 10] = 31 << 5;  // full green color
        _paletteRam[16 * 15 + 11] = 31 << 10;  // full blue color
        _paletteRam[16 * 15 + 12] = 31 + (31 << 5);  // red + green = yellow
        _paletteRam[16 * 15 + 13] = 31 + (31 << 10);  // red + blue = purple
        _paletteRam[16 * 15 + 14] = (31 << 5) + (31 << 10);  // green + blue = turquoise
        _paletteRam[16 * 15 + 15] = (31 << 10) + (31 << 5) + 31;  // full white color
    }

    void TextBGManager::clear() {
        memset(_mapRam, 0, 2 * 32 * 32);
        _tileReserve = 1;
    }

    void TextBGManager::clearRect(int x, int y, int w, int h) {
        // Look at drawGlyph for an explanation on this code
        // as it follows the same idea.
        int dstY = y + h;
        for (;y < dstY; ) {
            int x_ = x;
            for (;x_ < x + w;) {
                u8* tilePointer = getTile(x_, y);
                u8 tileY = y % 8;
                u8 tileX = x_ % 8;
                u8* tileByte = tilePointer + (((tileY * 8 + tileX) / 2) & (~1));
                auto* tile = (u16*) tileByte;

                if (tileX == 0 && tileY == 0 && x_ + 8 < x + w && y + 8 < dstY) {
                    x_ += 8;
                    memset(tilePointer, 0, 32);
                    continue;
                }
                for (;tileY < 8 && y / 8 + tileY < dstY; tileY++) {
                    int tileX_ = tileX;
                    for (;tileX_ < 8 && x_ / 8 + tileX_ < x + w; tileX_++) {
                        bool highBits = (tileX_ & 1) == 1;
                        bool prevByte = (((tileY * 8 + tileX_) / 2) & 1) == 1;

                        *tile &= ~(0xF << (4 * highBits) << (8 * prevByte));
                    }
                }
                x_ += 8 - (x_ % 8);
            }
            y += 8 - (y % 8);
        }
    }

    u8* TextBGManager::getTile(int x, int y) {
        x /= 8;
        y /= 8;
        u16 tileId = *((u8 *) _mapRam + (y * 32 + x) * 2);
        if (tileId == 0) {
            tileId = _tileReserve++;
            *(u16*)((u8 *) _mapRam + (y * 32 + x) * 2) = (15 << 12) + tileId;
            memset(((u8*)_tileRam) + (tileId * 32), 0, 32); // Initialize tile to blank
        }
        return ((u8*)_tileRam) + (tileId * 32);
    }

    void TextBGManager::setPaletteColor(int colorIdx, int r, int g, int b, bool color8bit) {
        if (color8bit) {
            r >>= 3;
            g >>= 3;
            b >>= 3;
        }
        _paletteRam[16 * 15 + colorIdx] = (b << 10) + (g << 5) + r;
    }

    void TextBGManager::setPaletteColor(int colorIdx, u16 color5bit) {
        _paletteRam[16 * 15 + colorIdx] = color5bit;
    }

    TextBGManager textMain(BG_PALETTE, BG_TILE_RAM(5), BG_MAP_RAM(4));
    TextBGManager textSub(BG_PALETTE_SUB, BG_TILE_RAM_SUB(5), BG_MAP_RAM_SUB(1));
}
