//
// Created by cervi on 25/08/2022.
//

#include "Sprite3DManager.hpp"

namespace Engine {
    int Sprite3DManager::loadSprite(Engine::Sprite &res) {
        if (!res.loaded)
            return -1;
        if (res.memory.allocated != NoAlloc)
            return -2;

        res.memory.paletteIdx = 96;
        for (int i = 0; i < 96; i++) {
            if (!paletteUsed[i]) {
                res.memory.paletteIdx = i;
                paletteUsed[i] = true;
                break;
            }
        }
        if (res.memory.paletteIdx == 96) {
            // no palette found
            return -3;
        }

        // copy palette
        vramSetBankE(VRAM_E_LCD);
        char buffer[100];
        u16* paletteBase = (u16*) ((u8*) VRAM_E + (256 * res.memory.paletteIdx + 1) * 2);
        dmaCopyHalfWords(3, res.sprite->getColors(), paletteBase, res.sprite->getColorCount() * 2);
        vramSetBankE(VRAM_E_TEX_PALETTE);

        res.memory.allocX = 8, res.memory.allocY = 8;
        uint8_t tileWidth, tileHeight;
        res.sprite->getSizeTiles(tileWidth, tileHeight);
        while (res.memory.allocX < tileWidth * 8)
            res.memory.allocX <<= 1;
        while (res.memory.allocY < tileHeight * 8)
            res.memory.allocY <<= 1;

        uint16_t neededTiles = res.memory.allocX * res.memory.allocY;
        int freeZoneIdx = 0;
        uint16_t start = 0;
        uint16_t length = 0;
        for (; freeZoneIdx < tileFreeZoneCount; freeZoneIdx++) {
            start = tileFreeZones[freeZoneIdx * 2];
            length = tileFreeZones[freeZoneIdx * 2 + 1];
            if (length >= neededTiles) {
                break;
            }
        }
        if (freeZoneIdx >= tileFreeZoneCount) {
            sprintf(buffer, "-2 start %d length %d needed %d",
                    start, length, neededTiles);
            nocashMessage(buffer);
            return -4;
        }

        if (length == neededTiles) {
            nocashMessage("alloc change");
            // Remove free zone
            tileFreeZoneCount--;
            auto* newFreeZones = new uint16_t[tileFreeZoneCount * 2];
            // Copy free zones up to freeZoneIdx
            memcpy(newFreeZones, tileFreeZones, freeZoneIdx * 4);
            // Copy free zones after freeZoneIdx
            memcpy((uint8_t*)newFreeZones + freeZoneIdx * 4,
                   (uint8_t*)tileFreeZones + (freeZoneIdx + 1) * 4,
                   (tileFreeZoneCount - freeZoneIdx) * 4);
            // Free old tileZones and change reference
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }
        else {
            tileFreeZones[freeZoneIdx * 2] += neededTiles;
            tileFreeZones[freeZoneIdx * 2 + 1] -= neededTiles;
            sprintf(buffer, "reduce change idx %d start %d length %d",
                    freeZoneIdx, tileFreeZones[freeZoneIdx * 2], tileFreeZones[freeZoneIdx * 2 + 1]);
            nocashMessage(buffer);
        }

        res.memory.tileStart = start;
        vramSetBankB(VRAM_B_LCD);
        uint8_t *tileRamStart = (uint8_t *) VRAM_B + res.memory.tileStart;
        memset(tileRamStart, 0, neededTiles);
        vramSetBankB(VRAM_B_TEXTURE_SLOT0);

        auto** activeSpriteNew = new Sprite*[activeSpriteCount + 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * activeSpriteCount);
        activeSpriteNew[activeSpriteCount] = &res;
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount++;

        res.memory.allocated = Allocated3D;
        res.memory.loadedFrame = -1;
        return 0;
    }

    void Sprite3DManager::freeSprite(Engine::Sprite &spr) {
        if (spr.memory.allocated != Allocated3D)
            return;
        int sprIdx = -1;
        for (int i = 0; i < activeSpriteCount; i++) {
            if (&spr == activeSprites[i]) {
                sprIdx = i;
                break;
            }
        }
        if (sprIdx == -1)
            return;

        paletteUsed[spr.memory.paletteIdx] = false;

        uint16_t start = spr.memory.tileStart;
        uint16_t length = spr.memory.allocX * spr.memory.allocY;

        int freeAfterIdx = 0;
        for (; freeAfterIdx < tileFreeZoneCount; freeAfterIdx++) {
            if (tileFreeZones[freeAfterIdx * 2] > start) {
                break;
            }
        }

        bool mergePrev = false, mergePost = false;

        if (freeAfterIdx > 0)
            mergePrev = (tileFreeZones[freeAfterIdx * 2 - 2] + tileFreeZones[freeAfterIdx * 2 -1]) == start;
        if (freeAfterIdx <= tileFreeZoneCount - 1) {
            mergePost = (start + length) == tileFreeZones[freeAfterIdx * 2];
        }

        if (mergePost && mergePrev)
        {
            tileFreeZoneCount--;
            auto* newFreeZones = new uint16_t[tileFreeZoneCount * 2];
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[(freeAfterIdx - 1) * 2 + 1] += length + tileFreeZones[freeAfterIdx * 2 + 1];
            memcpy((uint8_t*)newFreeZones + freeAfterIdx * 4,
                   (uint8_t*)tileFreeZones + (freeAfterIdx + 1) * 4,
                   (tileFreeZoneCount - freeAfterIdx) * 4);
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }
        else if (mergePrev)
        {
            tileFreeZones[(freeAfterIdx - 1) * 2 + 1] += length;
        }
        else if (mergePost)
        {
            tileFreeZones[freeAfterIdx * 2] -= length;
            tileFreeZones[freeAfterIdx * 2 + 1] += length;
        }
        else
        {
            tileFreeZoneCount++;
            auto* newFreeZones = new uint16_t[2 * tileFreeZoneCount];
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[freeAfterIdx * 2] = start;
            newFreeZones[freeAfterIdx * 2 + 1] = length;
            memcpy((uint8_t*)newFreeZones + (freeAfterIdx + 1) * 4,
                   tileFreeZones + freeAfterIdx * 4,
                   (tileFreeZoneCount - (freeAfterIdx + 1)) * 4);
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }

        auto** activeSpriteNew = new Sprite*[activeSpriteCount - 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &activeSprites[sprIdx + 1],
               sizeof(Sprite**) * (activeSpriteCount - sprIdx - 1));
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount--;

        spr.memory.allocated = NoAlloc;
    }

    int Sprite3DManager::loadSpriteFrame(Engine::Sprite &spr, int frame) {
        if (spr.memory.loadedFrame == frame)
            return -1;
        if (frame >= spr.sprite->getFrameCount() || frame < 0)
            return -2;
        spr.memory.loadedFrame = frame;

        uint8_t *tileRamStart = (uint8_t *) VRAM_B + spr.memory.tileStart;

        uint8_t tileWidth, tileHeight;
        spr.sprite->getSizeTiles(tileWidth, tileHeight);
        for (int y = 0; y < tileHeight * 8; y++) {
            for (int x = 0; x < tileWidth * 8; x++) {
                int tileX = x / 8;
                int tileY = y / 8;

                uint16_t framePos = frame * tileWidth * tileHeight;
                uint32_t tileOffset = framePos + tileY * tileWidth + tileX;
                tileOffset *= 64;
                tileOffset += (y % 8) * 8 + (x % 8);
                *(uint16_t*)(tileRamStart + y * spr.memory.allocX + x) &= ~(0xFF << (8 * (x & 1)));
                *(uint16_t*)(tileRamStart + y * spr.memory.allocX + x) |= (spr.sprite->getTiles()[tileOffset] & 0xFF) << (8 * (x & 1));
            }
        }
        return 0;
    }

    void Sprite3DManager::draw() {
        for (int i = 0; i < activeSpriteCount; i++) {
            Sprite* spr = activeSprites[i];

            spr->tick();

            if (spr->memory.loadedFrame == -1)
                continue;

            glColor( RGB15(31,31,31) );
            glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE);

            uint8_t allocXFmt = 0;
            for (int x = spr->memory.allocX; x > 8; x >>= 1) {
                allocXFmt += 1;
            }
            uint8_t allocYFmt = 0;
            for (int x = spr->memory.allocY; x > 8; x >>= 1) {
                allocYFmt += 1;
            }

            MATRIX_CONTROL = GL_MODELVIEW;
            MATRIX_IDENTITY = 0;
            uint8_t tileWidth, tileHeight;
            spr->sprite->getSizeTiles(tileWidth, tileHeight);
            uint32_t x = spr->x >> 8;
            uint32_t x2 = x + ((tileWidth * 8 * spr->scale_x) >> 8);
            uint32_t w = tileWidth * 8;
            uint32_t y = (spr->y >> 8);
            uint32_t y2 = y + ((tileHeight * 8 * spr->scale_y) >> 8);
            uint32_t h = tileHeight * 8;
            GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) + (4 << 26) + (1 << 29) + spr->memory.tileStart / 8;
            GFX_PAL_FORMAT = spr->memory.paletteIdx * 2 * 256 / 16;
            GFX_BEGIN = GL_QUADS;
            GFX_TEX_COORD = 0;
            GFX_VERTEX16 = x + (y << 16);
            GFX_VERTEX16 = spr->layer;
            GFX_TEX_COORD = h << (4 + 16);
            GFX_VERTEX_XY = x + (y2 << 16);
            GFX_TEX_COORD = (h << (4 + 16)) + (w << 4);
            GFX_VERTEX_XY = x2 + (y2 << 16);
            GFX_TEX_COORD = (w << 4);
            GFX_VERTEX_XY = x2 + (y << 16);
            GFX_END = 0;
        }
    }

    void Sprite3DManager::updateTextures() {
        vramSetBankB(VRAM_B_LCD);
        for (int i = 0; i < activeSpriteCount; i++) {
            Sprite* spr = activeSprites[i];

            if (spr->currentFrame != spr->memory.loadedFrame)
                loadSpriteFrame(*spr, spr->currentFrame);
        }
        vramSetBankB(VRAM_B_TEXTURE_SLOT0);
    }

    Sprite3DManager main3dSpr;
}
