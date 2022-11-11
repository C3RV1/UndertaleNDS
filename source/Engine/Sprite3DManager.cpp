//
// Created by cervi on 25/08/2022.
//

#include "Sprite3DManager.hpp"
#include "Texture.hpp"
#include "DEBUG_FLAGS.hpp"

int getOnesInBin(int x) {
    int count = 0;
    while (x > 0) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

namespace Engine {
    int Sprite3DManager::loadSprite(Engine::Sprite &res) {
        if (!res._loaded)
            return -1;
        if (res._memory.allocated != NoAlloc)
            return -2;

        auto** activeSpriteNew = new Sprite*[_activeSprCount + 1];
        memcpy(activeSpriteNew, _activeSpr, sizeof(Sprite**) * _activeSprCount);
        activeSpriteNew[_activeSprCount] = &res;
        delete[] _activeSpr;
        _activeSpr = activeSpriteNew;

        _activeSprCount++;

        res._memory.allocated = Allocated3D;
        res._memory.loadedFrame = -1;
        res._memory.loadedIntoMemory = false;
        return 0;
    }

    void Sprite3DManager::freeSprite(Engine::Sprite &spr) {
        if (spr._memory.allocated != Allocated3D)
            return;
        int sprIdx = -1;
        if (_activeSpr == nullptr)
            return;
        for (int i = 0; i < _activeSprCount; i++) {
            if (&spr == _activeSpr[i]) {
                sprIdx = i;
                break;
            }
        }
        if (sprIdx == -1)
            return;

        freeSpriteTexture(spr);

        auto** activeSpriteNew = new Sprite*[_activeSprCount - 1];
        memcpy(activeSpriteNew, _activeSpr, sizeof(Sprite**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &_activeSpr[sprIdx + 1],
               sizeof(Sprite**) * (_activeSprCount - sprIdx - 1));
        delete[] _activeSpr;
        _activeSpr = activeSpriteNew;

        _activeSprCount--;

        spr._memory.allocated = NoAlloc;
    }

    void Sprite3DManager::loadSpriteTexture(Engine::Sprite &spr) {
        spr._texture->_loaded3DCount += 1;
        if (spr._texture->_loaded3DCount > 1) { // Already loaded to texture
            spr._memory.loadedIntoMemory = true;
            return;
        }

        spr._texture->_color8bit = spr._texture->_colorCount >= 16;
        u16 length, alignment, tileBytes;
        if (spr._texture->_color8bit) {
            length = 16;
            alignment = 16;
            tileBytes = 64;
        } else {
            length = 1;
            alignment = 1;
            tileBytes = 32;
        }

        int res = paletteFreeZones.reserve(length, spr._texture->_paletteIdx, alignment);
        if (res != 0) {
            // no palette found
            return;
        }

        u16* paletteBase = &VRAM_E[16 * spr._texture->_paletteIdx + 1];
        dmaCopyHalfWordsAsynch(3, spr._texture->_colors, paletteBase,
                               spr._texture->_colorCount * 2);

        u8 tileWidth, tileHeight;
        spr._texture->getSizeTiles(tileWidth, tileHeight);

        int allocX = getOnesInBin(tileWidth);
        int allocY = getOnesInBin(tileHeight);
        spr._texture->_tileStart = new u16[allocX * allocY];

        int tileIdx = 0;

        int tilePosY = 0;
        int tileHeight_ = tileHeight;
        while (tileHeight_ > 0) {
            u8 subTileHeight = 1;
            while (subTileHeight << 1 <= tileHeight_)
                subTileHeight <<= 1;

            int tileWidth_ = tileWidth;
            int tilePosX = 0;
            while (tileWidth_ > 0) {
                u8 subTileWidth = 1;
                while (subTileWidth << 1 <= tileWidth_)
                    subTileWidth <<= 1;

                u16 neededTiles = subTileWidth * subTileHeight * spr._texture->_frameCount * tileBytes;
                if (tileFreeZones.reserve(neededTiles, spr._texture->_tileStart[tileIdx], 1) == 1) {
                    return;
                }

                for (int frame = 0; frame < spr._texture->_frameCount; frame++) {
                    for (int y = tilePosY * 8, y2 = 0; y < (tilePosY + subTileHeight) * 8; y++, y2++) {
                        for (int x = tilePosX * 8, x2 = 0; x < (tilePosX + subTileWidth) * 8; x++, x2++) {
                            int tileX = x / 8;
                            int tileY = y / 8;
                            u8 *tileRamStart = (u8 *) VRAM_B + spr._texture->_tileStart[tileIdx] +
                                               frame * subTileWidth * subTileHeight * tileBytes;

                            u16 framePos = frame * tileWidth * tileHeight;
                            u32 tileOffset = framePos + tileY * tileWidth + tileX;
                            tileOffset *= 64;
                            if (spr._texture->_color8bit) {
                                tileOffset += (y % 8) * 8 + (x % 8);
                                u16* dst = (u16*)(tileRamStart + y2 * subTileWidth * 8 + x2);
                                *dst &= ~(0xFF << (8 * (x2 & 1)));
                                *dst |= (spr._texture->_tiles[tileOffset] & 0xFF) << (8 * (x2 & 1));
                            } else {
                                tileOffset += (y % 8) * 8 + (x % 8);
                                u16* dst = (u16*)(tileRamStart + (y2 * subTileWidth * 8 + x2) / 2);
                                *dst &= ~(0xF << (4 * (x2 & 3)));
                                *dst |= (spr._texture->_tiles[tileOffset] & 0xFF) << (4 * (x2 & 3));
                            }
                        }
                    }
                }
                tileIdx++;
                tileWidth_ -= subTileWidth;
                tilePosX += subTileWidth;
            }
            tileHeight_ -= subTileHeight;
            tilePosY += subTileHeight;
        }

        spr._memory.loadedIntoMemory = true;
    }


    void Sprite3DManager::freeSpriteTexture(Engine::Sprite &spr) {
        spr._texture->_loaded3DCount -= 1;
        if (spr._texture->_loaded3DCount > 0) // Texture used by another sprite
            return;

        _paletteUsed[spr._texture->_paletteIdx] = false;

        u8 tileWidth, tileHeight;
        u8 tileBytes = spr._texture->_color8bit ? 64 : 32;
        spr._texture->getSizeTiles(tileWidth, tileHeight);

        int tileIdx = 0;
        while (tileHeight > 0) {
            u8 subTileHeight = 1;
            while (subTileHeight << 1 <= tileHeight)
                subTileHeight <<= 1;
            int tileWidth_ = tileWidth;
            while (tileWidth_ > 0) {
                u8 subTileWidth = 1;
                while (subTileWidth << 1 <= tileWidth_)
                    subTileWidth <<= 1;
                u16 neededTiles = subTileWidth * subTileHeight * spr._texture->_frameCount * tileBytes;
                tileFreeZones.free(neededTiles, spr._texture->_tileStart[tileIdx]);
                tileIdx++;
                tileWidth_ -= subTileWidth;
            }
            tileHeight -= subTileHeight;
        }

        delete[] spr._texture->_tileStart;
        spr._texture->_tileStart = nullptr;
    }

    void Sprite3DManager::draw() {
        if (_activeSpr == nullptr)
            return;
        for (int i = 0; i < _activeSprCount; i++) {
            Sprite* spr = _activeSpr[i];

            spr->tick();

            if (!spr->_memory.loadedIntoMemory)
                continue;

            glColor( RGB15(31,31,31) );
            glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE);

            u8 tileWidth, tileHeight;
            u8 tileFormat = spr->_texture->_color8bit ? 4 : 3;
            u8 tileBytes = spr->_texture->_color8bit ? 64 : 32;
            spr->_texture->getSizeTiles(tileWidth, tileHeight);

            int tileIdx = 0;
            int tilePosY = 0;
            while (tileHeight > 0) {
                u8 subTileHeight = 1;
                u8 allocYFmt = 0;
                while (subTileHeight << 1 <= tileHeight) {
                    subTileHeight <<= 1;
                    allocYFmt += 1;
                }

                int tileWidth_ = tileWidth;
                int tilePosX = 0;
                while (tileWidth_ > 0) {
                    u8 subTileWidth = 1;
                    u8 allocXFmt = 0;
                    while (subTileWidth << 1 <= tileWidth_) {
                        subTileWidth <<= 1;
                        allocXFmt += 1;
                    }

                    s32 x = ((spr->_x - (1 << 4)) >> 8) + 1;
                    x += (tilePosX * 8 * spr->_scale_x) >> 8;
                    s32 x2 = x + ((subTileWidth * 8 * spr->_scale_x) >> 8);
                    s32 w = subTileWidth * 8;
                    s32 y = (((spr->_y - (1 << 4)) >> 8)) + 1;
                    y += (tilePosY * 8 * spr->_scale_y) >> 8;
                    s32 y2 = y + ((subTileHeight * 8 * spr->_scale_y) >> 8);
                    s32 h = subTileHeight * 8;

                    if (x > 256 || x2 < 0 || y > 192 || y2 < 0) {
                        tileIdx++;
                        tileWidth_ -= subTileWidth;
                        tilePosX += subTileWidth;
                        continue;
                    }

                    MATRIX_CONTROL = GL_MODELVIEW;
                    MATRIX_IDENTITY = 0;
                    GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) + (tileFormat << 26) + (1 << 29) +
                                     (spr->_texture->_tileStart[tileIdx] + spr->_cFrame * subTileWidth * subTileHeight * tileBytes) / 8;
                    if (tileFormat == 4)
                        GFX_PAL_FORMAT = spr->_texture->_paletteIdx;
                    else
                        GFX_PAL_FORMAT = spr->_texture->_paletteIdx * 2;
                    GFX_BEGIN = GL_QUADS;
                    GFX_TEX_COORD = 0;
                    GFX_VERTEX16 = x + (y << 16);
                    GFX_VERTEX16 = spr->_layer + spr->_texture->_topDownOffset;
                    GFX_TEX_COORD = h << (4 + 16);
                    GFX_VERTEX_XY = x + (y2 << 16);
                    GFX_TEX_COORD = (h << (4 + 16)) + (w << 4);
                    GFX_VERTEX_XY = x2 + (y2 << 16);
                    GFX_TEX_COORD = (w << 4);
                    GFX_VERTEX_XY = x2 + (y << 16);
                    GFX_END = 0;

                    tileIdx++;
                    tileWidth_ -= subTileWidth;
                    tilePosX += subTileWidth;
                }
                tileHeight -= subTileHeight;
                tilePosY += subTileHeight;
            }
        }
    }

    void Sprite3DManager::updateTextures() {
        bool setBank = false;
        if (_activeSpr == nullptr)
            return;
        for (int i = 0; i < _activeSprCount; i++) {
            Sprite* spr = _activeSpr[i];

            if (!spr->_memory.loadedIntoMemory) {
#ifdef DEBUG_3D
                char buffer[100];
                sprintf(buffer, "Loading sprite %d out of %d", i + 1, activeSpriteCount);
                nocashMessage(buffer);
#endif
                if (!setBank) {
                    vramSetBankB(VRAM_B_LCD);
                    vramSetBankE(VRAM_E_LCD);
                    setBank = true;
                }
                loadSpriteTexture(*spr);
            }
        }
        if (setBank) {
            vramSetBankB(VRAM_B_TEXTURE_SLOT0);
            vramSetBankE(VRAM_E_TEX_PALETTE);
        }
    }

    Sprite3DManager main3dSpr;
}
