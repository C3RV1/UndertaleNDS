#include "Engine/OAMManager.hpp"
#include "Engine/Texture.hpp"
#include "DEBUG_FLAGS.hpp"

namespace Engine {
    int OAMManager::loadSprite(Sprite& res) {
        if (!res._loaded)
            return -1;
        if (res._memory.allocated != NoAlloc)
            return -2;

        res._memory.paletteColors = new u8[res._texture->_colorCount];
        u16* colors = res._texture->_colors;

        for (int i = 0; i < res._texture->_colorCount; i++) {
            int result = -1;
            bool foundColor = false;

            for (int paletteIdx = 0; paletteIdx < 255; paletteIdx++) {
                if (_paletteRam[1 + paletteIdx] == colors[i]) {
                    foundColor = true;
                    result = paletteIdx;
                    break;
                }
                if (_paletteRefCounts[paletteIdx] == 0 and result == -1) {
                    result = paletteIdx;
                }
            }

            res._memory.paletteColors[i] = result + 1;
            _paletteRefCounts[result]++;

            if (!foundColor) {
                _paletteRam[1 + result] = colors[i];
            }
        }

        // Reserve oam tiles
        u8 tileWidth, tileHeight;
        res._texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;
        res._memory.oamEntryCount = oamW * oamH;
        res._memory.oamEntries = new u8[res._memory.oamEntryCount];

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
                    delete res._memory.oamEntries;
                    res._memory.oamEntries = nullptr;
                    delete res._memory.paletteColors;
                    res._memory.paletteColors = nullptr;
                    return oamId - 2;
                }
                res._memory.oamEntries[oamY * oamW + oamX] = oamId;
            }
        }

        auto** activeSpriteNew = new Sprite*[_activeSprCount + 1];
        memcpy(activeSpriteNew, _activeSpr, sizeof(Sprite**) * _activeSprCount);
        activeSpriteNew[_activeSprCount] = &res;
        delete[] _activeSpr;
        _activeSpr = activeSpriteNew;

        _activeSprCount++;

        res._memory.allocated = AllocatedOAM;
        res._memory.loadedFrame = -1;
        res._memory.loadedIntoMemory = false;
        return 0;
    }

    int OAMManager::loadSpriteFrame(Engine::Sprite &spr, int frame) {
        if (spr._memory.loadedFrame == frame)
            return -1;
        if (frame >= spr._texture->_frameCount || frame < 0)
            return -2;
        spr._memory.loadedFrame = frame;
        u8 tileWidth, tileHeight;
        spr._texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;
        u16 framePos = frame * tileWidth * tileHeight;

        // Copy tile into memory (replacing colors)
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr._memory.oamEntries[oamY * oamW + oamX];
                OAMEntry* oamEntry = &_oamEntries[oamId];
                u16* tileRamStart = (u16*)((u8*) _tileRam + oamEntry->tileStart * 64);
                u8 neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;

                memset(tileRamStart, 0, neededTiles * 64);

                u8 tilesX = 8, tilesY = 8;
                if (oamX == oamW - 1)
                    tilesX = tileWidth - (oamW - 1) * 8;
                if (oamY == oamH - 1)
                    tilesY = tileHeight - (oamH - 1) * 8;

                for (int tileY = 0; tileY < tilesY; tileY++) {
                    for (int tileX = 0; tileX < tilesX; tileX++) {
                        u16 tileXPos = oamX * 8 + tileX;
                        u16 tileYPos = oamY * 8 + tileY;
                        u32 tileOffset = framePos + tileYPos * tileWidth + tileXPos;
                        tileOffset *= 64;
                        for (int pixelY = 0; pixelY < 8; pixelY++) {
                            for (int pixelX = 0; pixelX < 8; pixelX++) {
                                u32 resultOffset = (tileY * oamEntry->tileWidth + tileX) * 8 * 8 + pixelY * 8 + pixelX;
                                u32 tilesOffset = tileOffset + pixelY * 8 + pixelX;
                                u8 pixel = spr._texture->_tiles[tilesOffset];
                                if (pixel == 0) {
                                    continue;
                                }
                                tileRamStart[resultOffset / 2] += spr._memory.paletteColors[pixel - 1] << (8 * (resultOffset & 1));
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }

    int OAMManager::reserveOAMEntry(u8 tileW, u8 tileH) {
        int oamId = -1;
        OAMEntry* oamEntry = nullptr;
        for (int i = 0; i < SPRITE_COUNT; i++) {
            if (_oamEntries[i].free_) {
                oamId = i;
                oamEntry = &_oamEntries[i];
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
        _tileZones.reserve(neededTiles, oamEntry->tileStart, 1);
#ifdef DEBUG_2D
        dumpOamState();
#endif

        return oamId;
    }

    void OAMManager::freeOAMEntry(int oamId) {
        if (_oamEntries[oamId].free_)
            return;
        _oamEntries[oamId].free_ = true;

        OAMEntry* oamEntry = &_oamEntries[oamId];

        auto* oamStart = (u16*) ((u8*) _oamRam + oamId * 8);
        oamStart[0] = 1 << 9; // Not displayed
        oamStart[1] = 0;
        oamStart[2] = 0;

        u16 length = oamEntry->tileWidth * oamEntry->tileHeight;
        _tileZones.free(length, oamEntry->tileStart);

#ifdef DEBUG_2D
        dumpOamState();
#endif
    }

#ifdef DEBUG_2D
    void OAMManager::dumpOamState() {
        char buffer[100];
        for (int i = 0; i < 128; i++) {
            if (oamEntries[i].free_)
                continue;
            sprintf(buffer, "OAM %d start %d w %d h %d", i,
                    oamEntries[i].tileStart, oamEntries[i].tileWidth, oamEntries[i].tileHeight);
            nocashMessage(buffer);
        }
    }
#endif

    void OAMManager::freeSprite(Sprite& spr) {
        if (spr._memory.allocated != AllocatedOAM)
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

        for (int colorIdx = 0; colorIdx < spr._texture->_colorCount; colorIdx++) {
            u8 paletteColor = spr._memory.paletteColors[colorIdx];
            _paletteRefCounts[paletteColor - 1]--;
        }
        delete[] spr._memory.paletteColors;
        spr._memory.paletteColors = nullptr;

        if (spr._memory.oamScaleIdx != 0xff) {
            freeOamScaleEntry(spr);
            spr._memory.oamScaleIdx = 0xff;
        }
        for (int oamIdx = 0; oamIdx < spr._memory.oamEntryCount; oamIdx++) {
            u8 oamId = spr._memory.oamEntries[oamIdx];
            freeOAMEntry(oamId);
        }
        delete[] spr._memory.oamEntries;
        spr._memory.oamEntries = nullptr;

        auto** activeSpriteNew = new Sprite*[_activeSprCount - 1];
        memcpy(activeSpriteNew, _activeSpr, sizeof(Sprite**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &_activeSpr[sprIdx + 1],
               sizeof(Sprite**) * (_activeSprCount - sprIdx - 1));
        delete[] _activeSpr;
        _activeSpr = activeSpriteNew;

        _activeSprCount--;

        spr._memory.allocated = NoAlloc;
    }

    void OAMManager::draw() {
        if (_activeSpr == nullptr)
            return;
        for (int i = 0; i < _activeSprCount; i++) {
            Sprite* spr = _activeSpr[i];

            spr->tick();

            if (spr->_cFrame != spr->_memory.loadedFrame)
                loadSpriteFrame(*spr, spr->_cFrame);

            if (spr->_memory.loadedFrame == -1)
                continue;

            if (!spr->_memory.loadedIntoMemory)
                setOAMState(*spr);

            setSpritePosAndScale(*spr);
        }
    }

    void OAMManager::setOAMState(Engine::Sprite &spr) {
        for (int i = 0; i < spr._memory.oamEntryCount; i++) {
            u8 oamId = spr._memory.oamEntries[i];
            OAMEntry* oamEntry = &_oamEntries[oamId];
            auto* oamStart = (u16*) ((u8*) _oamRam + oamId * 8);
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
        spr._memory.loadedIntoMemory = true;
    }

    void OAMManager::setSpritePosAndScale(Engine::Sprite &spr) {
        u8 tileWidth, tileHeight;
        spr._texture->getSizeTiles(tileWidth, tileHeight);
        u8 oamW = (tileWidth + 7) / 8;
        u8 oamH = (tileHeight + 7) / 8;
        for (int oamY = 0; oamY < oamH; oamY++) {
            for (int oamX = 0; oamX < oamW; oamX++) {
                int oamId = spr._memory.oamEntries[oamY * oamW + oamX];
                auto* oamStart = (u16*) ((u8*) _oamRam + oamId * 8);
                bool useScale = (spr._scale_x != (1 << 8)) || (spr._scale_y != (1 << 8));
                if (useScale) {
                    if (spr._memory.oamScaleIdx == 0xff) {
                        allocateOamScaleEntry(spr);
                    }
                } else {
                    if (spr._memory.oamScaleIdx != 0xff) {
                        freeOamScaleEntry(spr);
                    }
                }

                oamStart[1] &= ~(0b1111 << 9);
                if (spr._memory.oamScaleIdx != 0xff) {
                    oamStart[0] |= 1 << 8;  // set scale and rotation flag
                    oamStart[0] |= 1 << 9;  // set scale and rotation flag
                    oamStart[1] |= spr._memory.oamScaleIdx << 9;
                    auto* oamScaleA = (u16*)((u8*)_oamRam + spr._memory.oamScaleIdx * 0x20 + 0x6);
                    auto* oamScaleB = (u16*)((u8*)_oamRam + spr._memory.oamScaleIdx * 0x20 + 0xE);
                    auto* oamScaleC = (u16*)((u8*)_oamRam + spr._memory.oamScaleIdx * 0x20 + 0x16);
                    auto* oamScaleD = (u16*)((u8*)_oamRam + spr._memory.oamScaleIdx * 0x20 + 0x1E);
                    *oamScaleA = (1 << 16) / spr._scale_x;
                    *oamScaleB = 0;
                    *oamScaleC = 0;
                    *oamScaleD = (1 << 16) / spr._scale_y;
                } else {
                    oamStart[0] &= ~(1 << 8);
                    oamStart[0] &= ~(1 << 9);
                }
                oamStart[0] &= ~0xFF;
                s32 posX = spr._x + oamX * 64 * spr._scale_x;
                posX %= (512 << 8);
                if (posX < 0)
                    posX = (512 << 8) + posX;
                s32 posY = spr._y + oamY * 64 * spr._scale_y;
                posY %= (256 << 8);
                if (posY < 0)
                    posY = (256 << 8) + posY;
                oamStart[0] |= posY >> 8;
                oamStart[1] &= ~0x1FF;
                oamStart[1] |= posX >> 8;
                oamStart[2] &= ~(3 << 10);
                oamStart[2] |= (spr._layer & 0b11) << 10;
                // TODO: Disable oam if out of screen
            }
        }
    }

    void OAMManager::allocateOamScaleEntry(Engine::Sprite &spr) {
        for (int i = 0; i < 32; i++) {
            if (!_oamScaleEntryUsed[i]) {
                spr._memory.oamScaleIdx = i;
                _oamScaleEntryUsed[i] = true;
                return;
            }
        }
    }

    void OAMManager::freeOamScaleEntry(Engine::Sprite &spr) {
        _oamScaleEntryUsed[spr._memory.oamScaleIdx] = false;
       spr._memory.oamScaleIdx = 0xff;
    }

    OAMManager OAMManagerSub(SPRITE_PALETTE_SUB, SPRITE_GFX_SUB, OAM_SUB);
}