//
// Created by cervi on 25/08/2022.
//

#include "Engine/Sprite3DManager.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Engine.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Engine/dma_async.hpp"
#include <algorithm>

namespace Engine {
    int Sprite3DManager::loadSprite(Engine::Sprite &res) {
        if (!res._loaded)
            return -1;
        if (res._memory.allocated != NoAlloc)
            return -2;
        if (!res._texture->_has3D) {
            std::string buffer = "Error loading spr #r" + res._texture->_path + "#x to 3D: Sprite doesn't have 3D chunk.";
            throw_(buffer);
        }

        _activeSpr.push_back(&res);

        res._memory.allocated = Allocated3D;
        res._memory.loadedFrame = -1;
        res._memory.loadedIntoMemory = false;
        return 0;
    }

    void Sprite3DManager::freeSprite(Engine::Sprite &spr) {
        if (spr._memory.allocated != Allocated3D)
            return;
        auto sprIdx = std::find(_activeSpr.begin(), _activeSpr.end(), &spr);
        if (sprIdx == _activeSpr.end())
            return;
        _activeSpr.erase(sprIdx);

        freeSpriteTexture(spr);

        spr._memory.allocated = NoAlloc;
    }

    void Sprite3DManager::loadSpriteTexture(Engine::Sprite &spr) {
        spr._texture->_loaded3DCount += 1;
        if (spr._texture->_loaded3DCount > 1) { // Already loaded to texture
            spr._memory.loadedIntoMemory = true;
            return;
        }

        bool color8bit = spr._texture->_colors.size() > 15;
        u16 length, alignment, tileBytes;
        if (color8bit) {
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
            std::string buffer = "Error loading spr #r" + spr._texture->_path + "#x to 3D: No available palettes.";
            throw_(buffer);
        }

        u16* paletteBase = &VRAM_E[16 * spr._texture->_paletteIdx + 1];
        dmaCopyHalfWordsAsync(3, &spr._texture->_colors[0], paletteBase,
                               spr._texture->_colors.size() * 2);

        int allocX = spr._texture->_3dChunk.tilesAllocX;
        int allocY = spr._texture->_3dChunk.tilesAllocY;
        spr._texture->_tileStart.resize(allocX * allocY);

        for (int tileY = 0; tileY < allocY; tileY++) {
            for (int tileX = 0; tileX < allocX; tileX++) {
                int tileIdx = tileY * allocX + tileX;
                auto & textureTile = spr._texture->_3dChunk.tiles[tileIdx];
                u8 tileWidth = textureTile.tileWidth;
                u8 tileHeight = textureTile.tileHeight;
                u16 neededBytes = tileWidth * tileHeight * spr._texture->_frameCount * tileBytes;
                if (tileFreeZones.reserve(neededBytes, spr._texture->_tileStart[tileIdx], 1) == 1) {
                    std::string buffer = "Error loading spr #r" + spr._texture->_path + "#x to 3D: Couldn't reserve tiles.";
                    throw_(buffer);
                }

                u8 *tileRamStart = (u8 *) VRAM_B + spr._texture->_tileStart[tileIdx];
                dmaCopyWordsAsync(3, &textureTile.tileFrameData[0],
                                   tileRamStart, neededBytes);
            }
        }

        spr._memory.loadedIntoMemory = true;
    }


    void Sprite3DManager::freeSpriteTexture(Engine::Sprite &spr) {
        spr._texture->_loaded3DCount -= 1;
        if (spr._texture->_loaded3DCount > 0) // Texture used by another sprite
            return;

        _paletteUsed[spr._texture->_paletteIdx] = false;

        u8 tileBytes = spr._texture->_colors.size() > 15 ? 64 : 32;

        int allocX = spr._texture->_3dChunk.tilesAllocX;
        int allocY = spr._texture->_3dChunk.tilesAllocY;

        for (int tileY = 0; tileY < allocY; tileY++) {
            for (int tileX = 0; tileX < allocX; tileX++) {
                int tileIdx = tileY * allocX + tileX;
                auto & textureTile = spr._texture->_3dChunk.tiles[tileIdx];
                u8 tileWidth = textureTile.tileWidth;
                u8 tileHeight = textureTile.tileHeight;
                u16 neededBytes = tileWidth * tileHeight * spr._texture->_frameCount * tileBytes;
                tileFreeZones.free(neededBytes, spr._texture->_tileStart[tileIdx]);
            }
        }

        spr._texture->_tileStart.clear();
    }

    void Sprite3DManager::draw() {
        if (_activeSpr.empty())
            return;
        for (auto const & spr : _activeSpr) {
            spr->tick();

            if (!spr->_memory.loadedIntoMemory)
                continue;

            glColor( RGB15(31,31,31) );
            glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE);

            u8 tileFormat = spr->_texture->_colors.size() > 15 ? 4 : 3;
            u8 tileBytes = spr->_texture->_colors.size() > 15 ? 64 : 32;

            int allocX = spr->_texture->_3dChunk.tilesAllocX;
            int allocY = spr->_texture->_3dChunk.tilesAllocY;

            int tilePosY = 0;
            for (int tileY = 0; tileY < allocY; tileY++) {
                int tilePosX = 0;
                int tileH = 0;
                for (int tileX = 0; tileX < allocX; tileX++) {
                    int tileIdx = tileY * allocX + tileX;
                    auto & textureTile = spr->_texture->_3dChunk.tiles[tileIdx];
                    tileH = textureTile.tileHeight;

                    s32 x = ((spr->_x - (1 << 4)) >> 8) + 1;
                    x += (tilePosX * 8 * spr->_scale_x) >> 8;
                    s32 x2 = x + ((textureTile.tileWidth * 8 * spr->_scale_x) >> 8);
                    s32 w = textureTile.tileWidth * 8;
                    s32 y = (((spr->_y - (1 << 4)) >> 8)) + 1;
                    y += (tilePosY * 8 * spr->_scale_y) >> 8;
                    s32 y2 = y + ((textureTile.tileHeight * 8 * spr->_scale_y) >> 8);
                    s32 h = textureTile.tileHeight * 8;

                    if (x > 256 || x2 < 0 || y > 192 || y2 < 0) {
                        tilePosX += textureTile.tileWidth;
                        continue;
                    }

                    int allocXFmt = 0;
                    int allocYFmt = 0;
                    for (;1 << allocXFmt < textureTile.tileWidth; allocXFmt++);
                    for (;1 << allocYFmt < textureTile.tileHeight; allocYFmt++);

                    MATRIX_CONTROL = GL_MODELVIEW;
                    MATRIX_IDENTITY = 0;
                    GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) + (tileFormat << 26) + (1 << 29) +
                                     (spr->_texture->_tileStart[tileIdx] + spr->_cFrame * textureTile.tileWidth * textureTile.tileHeight * tileBytes) / 8;
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
                    tilePosX += textureTile.tileWidth;
                }
                tilePosY += tileH;
            }
        }
    }

    void Sprite3DManager::updateTextures() {
        bool setBank = false;
        if (_activeSpr.empty())
            return;
        for (auto const & spr : _activeSpr) {
            if (!spr->_memory.loadedIntoMemory) {
#ifdef DEBUG_3D
                nocashMessage("Loading sprite");
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
