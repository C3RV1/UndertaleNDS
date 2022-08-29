#include "Engine/OAMManager.hpp"

namespace Engine {
    int OAMManager::loadSprite(SpriteManager& res) {
        if (!res.loaded)
            return -1;
        if (res.memory.allocated != NoAlloc)
            return -2;

        res.memory.paletteColors = new uint8_t[res.sprite->getColorCount()];
        uint16_t* colors = res.sprite->getColors();

        for (int i = 0; i < res.sprite->getColorCount(); i++) {
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
        uint8_t tileWidth, tileHeight;
        res.sprite->getSizeTiles(tileWidth, tileHeight);
        uint8_t oamW = (tileWidth + 7) / 8;
        uint8_t oamH = (tileHeight + 7) / 8;
        res.memory.oamEntryCount = oamW * oamH;
        res.memory.oamEntries = new uint8_t[res.memory.oamEntryCount];

        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                uint8_t reserveX = 8, reserveY = 8;
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

        int loadResult = loadSpriteFrame(res, 0);
        if (loadResult < 0) {
            delete res.memory.oamEntries;
            res.memory.oamEntries = nullptr;
            delete res.memory.paletteColors;
            res.memory.paletteColors = nullptr;
            return loadResult - 4;
        }

        auto** activeSpriteNew = new SpriteManager*[activeSpriteCount + 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(SpriteManager**) * activeSpriteCount);
        activeSpriteNew[activeSpriteCount] = &res;
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount++;

        res.memory.allocated = AllocatedOAM;
        res.memory.loadedFrame = -1;
        return 0;
    }

    int OAMManager::loadSpriteFrame(Engine::SpriteManager &spr, int frame) {
        if (spr.memory.loadedFrame == frame)
            return -1;
        if (frame >= spr.sprite->getFrameCount() || frame < 0)
            return -2;
        spr.memory.loadedFrame = frame;
        uint8_t tileWidth, tileHeight;
        spr.sprite->getSizeTiles(tileWidth, tileHeight);
        uint8_t oamW = (tileWidth + 7) / 8;
        uint8_t oamH = (tileHeight + 7) / 8;

        auto* tmpRam = new uint8_t[64*64];

        // Copy tile into memory (replacing colors)
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr.memory.oamEntries[oamY * oamW + oamX];
                auto* oamStart = (uint16_t*) ((uint8_t*) oamRam + oamId * 8);
                oamStart[0] += 64 * oamY;
                oamStart[1] += 64 * oamX;
                OAMEntry* oamEntry = &oamEntries[oamId];
                uint8_t* tileRamStart = (uint8_t*) tileRam + oamEntry->tileStart * 8 * 8;
                uint8_t neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;

                memset(tmpRam, 0, neededTiles * 64);

                uint8_t tilesX = 8, tilesY = 8;
                if (oamX == oamW - 1)
                    tilesX = tileWidth - (oamW - 1) * 8;
                if (oamY == oamH - 1)
                    tilesY = tileHeight - (oamH - 1) * 8;

                for (int tileY = 0; tileY < tilesY; tileY++) {
                    for (int tileX = 0; tileX < tilesX; tileX++) {
                        uint16_t framePos = frame * tileWidth * tileHeight;
                        uint16_t tileXPos = oamX * 8 + tileX;
                        uint16_t tileYPos = oamY * 8 + tileY;
                        uint32_t tileOffset = framePos + tileYPos * tileWidth + tileXPos;
                        tileOffset = tileOffset * 8 * 8;
                        for (int pixelY = 0; pixelY < 8; pixelY++) {
                            for (int pixelX = 0; pixelX < 8; pixelX++) {
                                uint32_t resultOffset = (tileY * oamEntry->tileWidth + tileX) * 8 * 8 + pixelY * 8 + pixelX;
                                uint32_t tilesOffset = tileOffset + pixelY * 8 + pixelX;
                                uint8_t pixel = spr.sprite->getTiles()[tilesOffset];
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
        delete[] tmpRam;
        return 0;
    }

    int OAMManager::reserveOAMEntry(uint8_t tileW, uint8_t tileH) {
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
            nocashMessage("-1");
            return -1;
        }
        oamEntry->free_ = false;
        oamEntry->tileWidth = tileW;
        oamEntry->tileHeight = tileH;

        // load tiles in groups of animations
        uint16_t neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;
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
            char buffer[100];
            sprintf(buffer, "-2 start %d length %d needed %d",
                    start, length, neededTiles);
            nocashMessage(buffer);
            return -2;
        }

        if (length == neededTiles) {
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
        }

        oamEntry->tileStart = start;

        auto* oamStart = (uint16_t*) ((uint8_t*) oamRam + oamId * 8);
        oamStart[0] = 1 << 13; // set 256 color mode
        oamStart[1] = 0;
        oamStart[2] = start + (0 << 10);  // set start tile and priority 0
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

        return oamId;
    }

    void OAMManager::freeOAMEntry(int oamId) {
        if (oamEntries[oamId].free_)
            return;
        oamEntries[oamId].free_ = true;

        OAMEntry* oamEntry = &oamEntries[oamId];

        auto* oamStart = (uint16_t*) ((uint8_t*) oamRam + oamId * 8);
        oamStart[0] = 1 << 9; // Not displayed
        oamStart[1] = 0;
        oamStart[2] = 0;

        uint16_t start = oamEntry->tileStart;
        uint16_t length = oamEntry->tileWidth * oamEntry->tileHeight;

        int freeAfterIdx = 0;
        for (; freeAfterIdx < tileFreeZoneCount; freeAfterIdx++) {
            if (tileFreeZones[freeAfterIdx * 2] > start) {
                break;
            }
        }

        bool mergePrev = false, mergePost = false;

        if (freeAfterIdx > 0)
            mergePrev = (tileFreeZones[(freeAfterIdx - 1) * 2] + tileFreeZones[freeAfterIdx * 2 -1]) == start;
        if (freeAfterIdx <= tileFreeZoneCount - 1)
            mergePost = (start + length) == tileFreeZones[freeAfterIdx * 2];

        if (mergePost && mergePrev)
        {
            tileFreeZoneCount--;
            auto* newFreeZones = new uint16_t[2 * tileFreeZoneCount];
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
            newFreeZones[freeAfterIdx * 2 + 1] = start;
            memcpy((uint8_t*)newFreeZones + (freeAfterIdx + 1) * 4,
                   tileFreeZones + freeAfterIdx * 4,
                   (tileFreeZoneCount - (freeAfterIdx + 1)) * 4);
            delete[] tileFreeZones;
            tileFreeZones = newFreeZones;
        }
    }

    void OAMManager::freeSprite(SpriteManager& spr) {
        if (spr.memory.allocated != AllocatedOAM)
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

        for (int colorIdx = 0; colorIdx < spr.sprite->getColorCount(); colorIdx++) {
            uint8_t paletteColor = spr.memory.paletteColors[colorIdx];
            paletteRefCounts[paletteColor]--;
        }
        delete[] spr.memory.paletteColors;
        spr.memory.paletteColors = nullptr;

        for (int oamIdx = 0; oamIdx < spr.memory.oamEntryCount; oamIdx++) {
            uint8_t oamId = spr.memory.oamEntries[oamIdx];
            freeOAMEntry(oamId);
        }
        delete[] spr.memory.oamEntries;
        spr.memory.oamEntries = nullptr;

        auto** activeSpriteNew = new SpriteManager*[activeSpriteCount - 1];
        memcpy(activeSpriteNew, activeSprites, sizeof(SpriteManager**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &activeSprites[sprIdx + 1],
               sizeof(SpriteManager**) * (activeSpriteCount - sprIdx - 1));
        delete[] activeSprites;
        activeSprites = activeSpriteNew;

        activeSpriteCount--;

        spr.memory.allocated = NoAlloc;
    }

    void OAMManager::draw() {
        for (int i = 0; i < activeSpriteCount; i++) {
            SpriteManager* spr = activeSprites[i];

            if (spr->currentAnimation > 0) {
                CSPRAnimation* current = &spr->sprite->getAnims()[spr->currentAnimation];
                if (current->frames[spr->currentAnimationFrame].duration != 0) {
                    spr->currentAnimationTimer--;
                    if (spr->currentAnimationTimer == 0) {
                        spr->currentAnimationFrame++;
                        spr->currentAnimationFrame %= current->frameCount;
                        spr->currentFrame = current->frames[spr->currentAnimationFrame].frame;
                        spr->currentAnimationTimer = current->frames[spr->currentAnimationFrame].duration;
                    }
                }
            }

            if (spr->currentFrame != spr->memory.loadedFrame)
                loadSpriteFrame(*spr, spr->currentFrame);

            setSpritePos(*spr, spr->x >> 8, spr->y >> 8);
        }
    }

    void OAMManager::setSpritePos(Engine::SpriteManager &spr, int x, int y) {
        uint8_t tileWidth, tileHeight;
        spr.sprite->getSizeTiles(tileWidth, tileHeight);
        uint8_t oamW = (tileWidth + 7) / 8;
        uint8_t oamH = (tileHeight + 7) / 8;
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr.memory.oamEntries[oamY * oamW + oamX];
                auto* oamStart = (uint16_t*) ((uint8_t*) oamRam + oamId * 8);
                oamStart[0] &= ~0xFF;
                oamStart[0] |= y + oamY * 64;
                oamStart[1] &= ~0x1FF;
                oamStart[1] |= x + oamX * 64;
                // TODO: Disable sprite if out of screen
            }
        }
    }

    OAMManager OAMManagerSub(SPRITE_PALETTE_SUB, SPRITE_GFX_SUB, OAM_SUB);
}