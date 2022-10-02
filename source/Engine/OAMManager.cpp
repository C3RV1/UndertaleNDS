#include "Engine/OAMManager.hpp"
#include "Engine/Texture.hpp"
#include "DEBUG_FLAGS.hpp"

namespace Engine {
    u8 tmpRam[64*64];

    int OAMManager::loadSprite(Sprite& res) {
        if (!res.loaded)
            return -1;
        if (res.memory.allocated != NoAlloc)
            return -2;

        res.memory.paletteColors = new u8[res.texture->getColorCount()];
        u16* colors = res.texture->getColors();

        for (int i = 0; i < res.texture->getColorCount(); i++) {
            int result = -1;
            bool foundColor = false;

            for (int paletteIdx = 0; paletteIdx < 255; paletteIdx++) {
                if (paletteRam[1 + paletteIdx] == colors[i]) {
                    foundColor = true;
                    result = paletteIdx;
                    break;
                }
                if (paletteRefCounts[paletteIdx] == 0 and result == -1) {
                    result = paletteIdx;
                }
            }

            res.memory.paletteColors[i] = result + 1;
            paletteRefCounts[result]++;

            if (!foundColor) {
                paletteRam[1 + result] = colors[i];
            }
        }

        // Reserve oam tiles
        u8 tileWidth, tileHeight;
        res.texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;
        res.memory.oamEntryCount = oamW * oamH;
        res.memory.oamEntries = new u8[res.memory.oamEntryCount];

        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                u8 reserveX = 8, reserveY = 8;
                if (oamX == oamW - 1)
                    reserveX = tileWidth - (oamW - 1) * 8;
                if (oamY == oamH - 1)
                    reserveY = tileHeight - (oamH - 1) * 8;
                if (reserveX > 4)
                    reserveX = 8;
                else if (reserveX > 2)
                    reserveX = 4;
                else if (reserveX > 1)
                    reserveX = 2;
                if (reserveY > 4)
                    reserveY = 8;
                else if (reserveY > 2)
                    reserveY = 4;
                else if (reserveY > 1)
                    reserveY = 2;
                if (reserveX == 8 && reserveY < 8)
                    reserveY = 4;
                if (reserveY == 8 && reserveX < 8)
                    reserveX = 4;
                int oamId = reserveOAMEntry(reserveX, reserveY);
                if (oamId < 0)
                {
                    delete res.memory.oamEntries;
                    res.memory.oamEntries = nullptr;
                    delete res.memory.paletteColors;
                    res.memory.paletteColors = nullptr;
                    return oamId - 2;
                }
                res.memory.oamEntries[oamY * oamW + oamX] = oamId;
            }
        }

        auto** activeSpriteNew = new Sprite*[activeSpriteCount + 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * activeSpriteCount);
        activeSpriteNew[activeSpriteCount] = &res;
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount++;

        res.memory.allocated = AllocatedOAM;
        res.memory.loadedFrame = -1;
        res.memory.loadedIntoMemory = false;
        return 0;
    }

    int OAMManager::loadSpriteFrame(Engine::Sprite &spr, int frame) {
        if (spr.memory.loadedFrame == frame)
            return -1;
        if (frame >= spr.texture->getFrameCount() || frame < 0)
            return -2;
        spr.memory.loadedFrame = frame;
        u8 tileWidth, tileHeight;
        spr.texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;

        // Copy tile into memory (replacing colors)
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr.memory.oamEntries[oamY * oamW + oamX];
                OAMEntry* oamEntry = &oamEntries[oamId];
                u8* tileRamStart = (u8*) tileRam + oamEntry->tileStart * 8 * 8;
                u8 neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;

                memset(tmpRam, 0, neededTiles * 64);

                u8 tilesX = 8, tilesY = 8;
                if (oamX == oamW - 1)
                    tilesX = tileWidth - (oamW - 1) * 8;
                if (oamY == oamH - 1)
                    tilesY = tileHeight - (oamH - 1) * 8;

                for (int tileY = 0; tileY < tilesY; tileY++) {
                    for (int tileX = 0; tileX < tilesX; tileX++) {
                        u16 framePos = frame * tileWidth * tileHeight;
                        u16 tileXPos = oamX * 8 + tileX;
                        u16 tileYPos = oamY * 8 + tileY;
                        u32 tileOffset = framePos + tileYPos * tileWidth + tileXPos;
                        tileOffset = tileOffset * 8 * 8;
                        for (int pixelY = 0; pixelY < 8; pixelY++) {
                            for (int pixelX = 0; pixelX < 8; pixelX++) {
                                u32 resultOffset = (tileY * oamEntry->tileWidth + tileX) * 8 * 8 + pixelY * 8 + pixelX;
                                u32 tilesOffset = tileOffset + pixelY * 8 + pixelX;
                                u8 pixel = spr.texture->getTiles()[tilesOffset];
                                if (pixel == 0) {
                                    tmpRam[resultOffset] = 0;
                                    continue;
                                }
                                tmpRam[resultOffset] = spr.memory.paletteColors[pixel - 1];
                            }
                        }
                    }
                }

                dmaCopyWords(3, tmpRam, tileRamStart, 8 * 8 * neededTiles);
            }
        }
        return 0;
    }

    int OAMManager::reserveOAMEntry(u8 tileW, u8 tileH) {
        int oamId = -1;
        OAMEntry* oamEntry = nullptr;
        for (int i = 0; i < SPRITE_COUNT; i++) {
            if (oamEntries[i].free_) {
                oamId = i;
                oamEntry = &oamEntries[i];
                break;
            }
        }
        if (oamId == -1) {
            nocashMessage("Oam full");
            return -1;
        }
        oamEntry->free_ = false;
        oamEntry->tileWidth = tileW;
        oamEntry->tileHeight = tileH;

        // load tiles in groups of animations
        u16 neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;
        int freeZoneIdx = 0;
        u16 start = 0;
        u16 length = 0;
        for (; freeZoneIdx < tileFreeZoneCount; freeZoneIdx++) {
            start = tileFreeZones[freeZoneIdx * 2];
            length = tileFreeZones[freeZoneIdx * 2 + 1];
            if (length >= neededTiles) {
                break;
            }
        }
        if (freeZoneIdx >= tileFreeZoneCount) {
            char buffer[100];
            sprintf(buffer, "2d alloc error start %d length %d needed %d",
                    start, length, neededTiles);
            nocashMessage(buffer);
            return -2;
        }

        if (length == neededTiles) {
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
            tileFreeZones[freeZoneIdx * 2] += neededTiles;
            tileFreeZones[freeZoneIdx * 2 + 1] -= neededTiles;
        }

        oamEntry->tileStart = start;
#ifdef DEBUG_SPRITES
        dumpOamState();
#endif

        return oamId;
    }

    void OAMManager::freeOAMEntry(int oamId) {
        if (oamEntries[oamId].free_)
            return;
        oamEntries[oamId].free_ = true;

        OAMEntry* oamEntry = &oamEntries[oamId];

        auto* oamStart = (u16*) ((u8*) oamRam + oamId * 8);
        oamStart[0] = 1 << 9; // Not displayed
        oamStart[1] = 0;
        oamStart[2] = 0;

        u16 start = oamEntry->tileStart;
        u16 length = oamEntry->tileWidth * oamEntry->tileHeight;

#ifdef DEBUG_SPRITES
        char buffer[100];
        sprintf(buffer, "2dfree start %d length %d oamId %d",
                start, length, oamId);
        nocashMessage(buffer);
#endif

        int freeAfterIdx = 0;
        for (; freeAfterIdx < tileFreeZoneCount; freeAfterIdx++) {
            if (tileFreeZones[freeAfterIdx * 2] > start) {
                break;
            }
        }

        bool mergePrev = false, mergePost = false;

        if (freeAfterIdx > 0)
            mergePrev = (tileFreeZones[freeAfterIdx * 2 - 2] + tileFreeZones[freeAfterIdx * 2 -1]) == start;
        if (freeAfterIdx <= tileFreeZoneCount - 1)
            mergePost = (start + length) == tileFreeZones[freeAfterIdx * 2];

        if (mergePost && mergePrev)
        {
            tileFreeZoneCount--;
            auto* newFreeZones = new u16[2 * tileFreeZoneCount];
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
#ifdef DEBUG_SPRITES
        dumpOamState();
#endif
    }

    void OAMManager::dumpOamState() {
        char buffer[100];
        for (int i = 0; i < tileFreeZoneCount; i++) {
            sprintf(buffer, "FREE ZONE %d (%d length)", tileFreeZones[i * 2],
                    tileFreeZones[i * 2 + 1]);
            nocashMessage(buffer);
        }
        for (int i = 0; i < 128; i++) {
            if (oamEntries[i].free_)
                continue;
            sprintf(buffer, "OAM %d start %d w %d h %d", i,
                    oamEntries[i].tileStart, oamEntries[i].tileWidth, oamEntries[i].tileHeight);
            nocashMessage(buffer);
        }
    }

    void OAMManager::freeSprite(Sprite& spr) {
        if (spr.memory.allocated != AllocatedOAM)
            return;
        int sprIdx = -1;
        if (activeSprites == nullptr)
            return;
        for (int i = 0; i < activeSpriteCount; i++) {
            if (&spr == activeSprites[i]) {
                sprIdx = i;
                break;
            }
        }
        if (sprIdx == -1)
            return;

        for (int colorIdx = 0; colorIdx < spr.texture->getColorCount(); colorIdx++) {
            u8 paletteColor = spr.memory.paletteColors[colorIdx];
            paletteRefCounts[paletteColor - 1]--;
        }
        delete[] spr.memory.paletteColors;
        spr.memory.paletteColors = nullptr;

        if (spr.memory.oamScaleIdx != 0xff) {
            freeOamScaleEntry(spr);
            spr.memory.oamScaleIdx = 0xff;
        }
        for (int oamIdx = 0; oamIdx < spr.memory.oamEntryCount; oamIdx++) {
            u8 oamId = spr.memory.oamEntries[oamIdx];
            freeOAMEntry(oamId);
        }
        delete[] spr.memory.oamEntries;
        spr.memory.oamEntries = nullptr;

        auto** activeSpriteNew = new Sprite*[activeSpriteCount - 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(Sprite**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &activeSprites[sprIdx + 1],
               sizeof(Sprite**) * (activeSpriteCount - sprIdx - 1));
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount--;

        spr.memory.allocated = NoAlloc;
    }

    void OAMManager::draw() {
        if (activeSprites == nullptr)
            return;
        for (int i = 0; i < activeSpriteCount; i++) {
            Sprite* spr = activeSprites[i];

            spr->tick();

            if (spr->currentFrame != spr->memory.loadedFrame)
                loadSpriteFrame(*spr, spr->currentFrame);

            if (spr->memory.loadedFrame == -1)
                continue;

            if (!spr->memory.loadedIntoMemory)
                setOAMState(*spr);

            setSpritePosAndScale(*spr);
        }
    }

    void OAMManager::setOAMState(Engine::Sprite &spr) {
        for (int i = 0; i < spr.memory.oamEntryCount; i++) {
            u8 oamId = spr.memory.oamEntries[i];
            OAMEntry* oamEntry = &oamEntries[oamId];
            auto* oamStart = (u16*) ((u8*) oamRam + oamId * 8);
            oamStart[0] = 1 << 13; // set 256 color mode
            oamStart[1] = 0;
            oamStart[2] = oamEntry->tileStart + (0 << 10);  // set start tile and priority 0
            // set size mode
            if (oamEntry->tileWidth == oamEntry->tileHeight) {
                switch (oamEntry->tileWidth) {
                    case 2:
                        oamStart[1] |= 1 << 14;
                        break;
                    case 4:
                        oamStart[1] |= 2 << 14;
                        break;
                    case 8:
                        oamStart[1] |= 3 << 14;
                        break;
                    default:
                        break;
                }
            }
            else if (oamEntry->tileWidth > oamEntry->tileHeight) {
                oamStart[0] |= 1 << 14;
                switch (oamEntry->tileWidth) {
                    case 4:
                        if (oamEntry->tileHeight == 1)
                            oamStart[1] |= 1 << 14;
                        else
                            oamStart[1] |= 2 << 14;
                        break;
                    case 8:
                        oamStart[1] |= 3 << 14;
                        break;
                    default:
                        break;
                }
            }
            else {
                oamStart[0] |= 2 << 14;
                switch (oamEntry->tileHeight) {
                    case 4:
                        if (oamEntry->tileWidth == 1)
                            oamStart[1] |= 1 << 14;
                        else
                            oamStart[1] |= 2 << 14;
                        break;
                    case 8:
                        oamStart[1] |= 3 << 14;
                        break;
                    default:
                        break;
                }
            }
        }
        spr.memory.loadedIntoMemory = true;
    }

    void OAMManager::setSpritePosAndScale(Engine::Sprite &spr) {
        u8 tileWidth, tileHeight;
        spr.texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr.memory.oamEntries[oamY * oamW + oamX];
                auto* oamStart = (u16*) ((u8*) oamRam + oamId * 8);
                bool useScale = (spr.scale_x != (1 << 8)) || (spr.scale_y != (1 << 8));
                if (useScale) {
                    if (spr.memory.oamScaleIdx == 0xff) {
                        allocateOamScaleEntry(spr);
                    }
                } else {
                    if (spr.memory.oamScaleIdx != 0xff) {
                        freeOamScaleEntry(spr);
                    }
                }

                oamStart[1] &= ~(0b1111 << 9);
                if (spr.memory.oamScaleIdx != 0xff) {
                    oamStart[0] |= 1 << 8;  // set scale and rotation flag
                    oamStart[1] |= spr.memory.oamScaleIdx << 9;
                    auto* oamScaleA = (u16*)((u8*)oamRam + spr.memory.oamScaleIdx * 0x20 + 0x6);
                    auto* oamScaleB = (u16*)((u8*)oamRam + spr.memory.oamScaleIdx * 0x20 + 0xE);
                    auto* oamScaleC = (u16*)((u8*)oamRam + spr.memory.oamScaleIdx * 0x20 + 0x16);
                    auto* oamScaleD = (u16*)((u8*)oamRam + spr.memory.oamScaleIdx * 0x20 + 0x1E);
                    *oamScaleA = (1 << 16) / spr.scale_x;
                    *oamScaleB = 0;
                    *oamScaleC = 0;
                    *oamScaleD = (1 << 16) / spr.scale_y;
                } else {
                    oamStart[0] &= ~(1 << 8);
                }
                oamStart[0] &= ~0xFF;
                s32 posX = spr.x + oamX * 64 * spr.scale_x;
                posX %= (512 << 8);
                if (posX < 0)
                    posX = (512 << 8) + posX;
                s32 posY = spr.y + oamY * 64 * spr.scale_y;
                posY %= (256 << 8);
                if (posY < 0)
                    posY = (256 << 8) + posY;
                oamStart[0] |= posY >> 8;
                oamStart[1] &= ~0x1FF;
                oamStart[1] |= posX >> 8;
                oamStart[2] &= ~(3 << 10);
                oamStart[2] |= (spr.layer & 0b11) << 10;
                // TODO: Disable oam if out of screen
            }
        }
    }

    void OAMManager::allocateOamScaleEntry(Engine::Sprite &spr) {
        for (int i = 0; i < 32; i++) {
            if (!oamScaleEntryUsed[i]) {
                spr.memory.oamScaleIdx = i;
                oamScaleEntryUsed[i] = true;
                return;
            }
        }
    }

    void OAMManager::freeOamScaleEntry(Engine::Sprite &spr) {
       oamScaleEntryUsed[spr.memory.oamScaleIdx] = false;
       spr.memory.oamScaleIdx = 0xff;
    }

    OAMManager OAMManagerSub(SPRITE_PALETTE_SUB, SPRITE_GFX_SUB, OAM_SUB);
}