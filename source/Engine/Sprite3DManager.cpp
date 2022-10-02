//
// Created by cervi on 25/08/2022.
//

#include "Sprite3DManager.hpp"
#include "Texture.hpp"

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

        u8 tileWidth, tileHeight;
        res.texture->getSizeTiles(tileWidth, tileHeight);
        res.memory.allocX = getOnesInBin(tileWidth);
        res.memory.allocY = getOnesInBin(tileHeight);
        res.memory.tileStart = new u16[res.memory.allocX * res.memory.allocY];
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
                u16 neededTiles = subTileWidth * subTileHeight * res.texture->getFrameCount() * 64;
                if (reserveTiles(neededTiles, res.memory.tileStart[tileIdx]) == 1) {
                    return -4;
                }
                tileIdx++;
                tileWidth_ -= subTileWidth;
            }
            tileHeight -= subTileHeight;
        }

        auto** activeSpriteNew = new Sprite*[activeSpriteCount + 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * activeSpriteCount);
        activeSpriteNew[activeSpriteCount] = &res;
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount++;

        res.memory.allocated = Allocated3D;
        res.memory.loadedFrame = -1;
        res.memory.loadedIntoMemory = false;
        return 0;
    }

    int Sprite3DManager::reserveTiles(u16 length, u16& start) {
        int freeZoneIdx = 0;
        u16 length_ = 0;
        for (; freeZoneIdx < tileFreeZoneCount; freeZoneIdx++) {
            start = tileFreeZones[freeZoneIdx * 2];
            length_ = tileFreeZones[freeZoneIdx * 2 + 1];
            if (length_ >= length) {
                break;
            }
        }
        if (freeZoneIdx >= tileFreeZoneCount) {
            char buffer[100];
            sprintf(buffer, "-2 start %d length %d needed %d",
                    start, length_, length);
            nocashMessage(buffer);
            return 1;
        }

        if (length == length_) {
            // Remove free zone
            tileFreeZoneCount--;
            auto* newFreeZones = new u16[tileFreeZoneCount * 2];
            // Copy free zones up to freeZoneIdx
            memcpy(newFreeZones, tileFreeZones, freeZoneIdx * 4);
            // Copy free zones after freeZoneIdx
            memcpy((u8*)newFreeZones + freeZoneIdx * 4,
                   (u8*)tileFreeZones + (freeZoneIdx + 1) * 4,
                   (tileFreeZoneCount - freeZoneIdx) * 4);
            // Free old tileZones and change reference
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }
        else {
            tileFreeZones[freeZoneIdx * 2] += length;
            tileFreeZones[freeZoneIdx * 2 + 1] -= length;
        }
        char buffer[100];
        sprintf(buffer, "3dalloc start %d length %d",
                start, length);
        nocashMessage(buffer);
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

        u8 tileWidth, tileHeight;
        spr.texture->getSizeTiles(tileWidth, tileHeight);
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
                u16 neededTiles = subTileWidth * subTileHeight * spr.texture->getFrameCount() * 64;
                freeTiles(neededTiles, spr.memory.tileStart[tileIdx]);
                tileIdx++;
                tileWidth_ -= subTileWidth;
            }
            tileHeight -= subTileHeight;
        }

        delete[] spr.memory.tileStart;

        auto** activeSpriteNew = new Sprite*[activeSpriteCount - 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &activeSprites[sprIdx + 1],
               sizeof(Sprite**) * (activeSpriteCount - sprIdx - 1));
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount--;

        spr.memory.allocated = NoAlloc;
    }

    void Sprite3DManager::freeTiles(u16 length, u16& start) {
        char buffer[100];
        sprintf(buffer, "3dfree start %d length %d",
                start, length);
        nocashMessage(buffer);

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
            auto* newFreeZones = new u16[tileFreeZoneCount * 2];
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[(freeAfterIdx - 1) * 2 + 1] += length + tileFreeZones[freeAfterIdx * 2 + 1];
            memcpy((u8*)newFreeZones + freeAfterIdx * 4,
                   (u8*)tileFreeZones + (freeAfterIdx + 1) * 4,
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
            auto* newFreeZones = new u16[2 * tileFreeZoneCount];
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[freeAfterIdx * 2] = start;
            newFreeZones[freeAfterIdx * 2 + 1] = length;
            memcpy((u8*)newFreeZones + (freeAfterIdx + 1) * 4,
                   tileFreeZones + freeAfterIdx * 4,
                   (tileFreeZoneCount - (freeAfterIdx + 1)) * 4);
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }
    }

    void Sprite3DManager::loadSpriteTexture(Engine::Sprite &spr) {
        u8 tileWidth, tileHeight;
        spr.texture->getSizeTiles(tileWidth, tileHeight);

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

                for (int frame = 0; frame < spr.texture->getFrameCount(); frame++) {
                    for (int y = tilePosY * 8, y2 = 0; y < (tilePosY + subTileHeight) * 8; y++, y2++) {
                        for (int x = tilePosX * 8, x2 = 0; x < (tilePosX + subTileWidth) * 8; x++, x2++) {
                            int tileX = x / 8;
                            int tileY = y / 8;
                            u8 *tileRamStart = (u8 *) VRAM_B + spr.memory.tileStart[tileIdx] +
                                               frame * subTileWidth * subTileHeight * 64;

                            u16 framePos = frame * tileWidth * tileHeight;
                            u32 tileOffset = framePos + tileY * tileWidth + tileX;
                            tileOffset *= 64;
                            tileOffset += (y % 8) * 8 + (x % 8);
                            *(u16 *) (tileRamStart + y2 * subTileWidth * 8 + x2) &= ~(0xFF << (8 * (x2 & 1)));
                            *(u16 *) (tileRamStart + y2 * subTileWidth * 8 + x2) |=
                                    (spr.texture->getTiles()[tileOffset] & 0xFF) << (8 * (x2 & 1));
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
    }

    void Sprite3DManager::draw() {
        for (int i = 0; i < activeSpriteCount; i++) {
            Sprite* spr = activeSprites[i];

            spr->tick();

            if (!spr->memory.loadedIntoMemory)
                continue;

            glColor( RGB15(31,31,31) );
            glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE);

            u8 tileWidth, tileHeight;
            spr->texture->getSizeTiles(tileWidth, tileHeight);

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

                    MATRIX_CONTROL = GL_MODELVIEW;
                    MATRIX_IDENTITY = 0;
                    u32 x = ((spr->x - (1 << 4)) >> 8) + 1;
                    x += (tilePosX * 8 * spr->scale_x) >> 8;
                    u32 x2 = x + ((subTileWidth * 8 * spr->scale_x) >> 8);
                    u32 w = subTileWidth * 8;
                    u32 y = (((spr->y - (1 << 4)) >> 8)) + 1;
                    y += (tilePosY * 8 * spr->scale_y) >> 8;
                    u32 y2 = y + ((subTileHeight * 8 * spr->scale_y) >> 8);
                    u32 h = subTileHeight * 8;
                    GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) + (4 << 26) + (1 << 29) +
                                     (spr->memory.tileStart[tileIdx] + spr->currentFrame * subTileWidth * subTileHeight * 64) / 8;
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
        char buffer[100];
        for (int i = 0; i < activeSpriteCount; i++) {
            Sprite* spr = activeSprites[i];

            if (!spr->memory.loadedIntoMemory) {
                sprintf(buffer, "Loading sprite %d out of %d", i + 1, activeSpriteCount);
                nocashMessage(buffer);
                spr->memory.loadedIntoMemory = true;
                if (!setBank) {
                    vramSetBankB(VRAM_B_LCD);
                    vramSetBankE(VRAM_E_LCD);
                    setBank = true;
                }
                loadPalette(*spr);
                loadSpriteTexture(*spr);
            }
        }
        if (setBank) {
            vramSetBankB(VRAM_B_TEXTURE_SLOT0);
            vramSetBankE(VRAM_E_TEX_PALETTE);
        }
    }

    void Sprite3DManager::loadPalette(Engine::Sprite &spr) {
        u16* paletteBase = (u16*) ((u8*) VRAM_E + (256 * spr.memory.paletteIdx + 1) * 2);
        dmaCopyHalfWords(3, spr.texture->getColors(), paletteBase, spr.texture->getColorCount() * 2);
    }

    Sprite3DManager main3dSpr;
}
