//
// Created by cervi on 25/08/2022.
//

#include "Sprite3DManager.hpp"

namespace Engine {
    int Sprite3DManager::loadSprite(Engine::Sprite &sprite, Engine::SpriteManager *&res) {
        if (!sprite.getLoaded())
            return -1;

        auto* sprEntry = (SpriteManager*) malloc(sizeof(SpriteManager));
        sprEntry->colorCount = sprite.getColorCount();
        sprEntry->frameCount = sprite.getFrameCount();
        sprEntry->currentFrame = -1;
        sprite.getSizeTiles(sprEntry->tileWidth, sprEntry->tileHeight);
        sprEntry->tileData = sprite.getTiles();
        sprEntry->animCount = sprite.getAnimCount();
        sprEntry->animations = sprite.getAnims();

        sprEntry->paletteIdx = 96;
        for (int i = 0; i < 96; i++) {
            if (!paletteUsed[i]) {
                sprEntry->paletteIdx = i;
                paletteUsed[i] = true;
                break;
            }
        }
        if (sprEntry->paletteIdx == 96) {
            // no palette found
            free(sprEntry);
            res = nullptr;
            return -2;
        }

        // copy palette
        vramSetBankE(VRAM_E_LCD);
        char buffer[100];
        u16* paletteBase = (u16*) ((u8*) VRAM_E + (256 * sprEntry->paletteIdx + 1) * 2);
        dmaCopyHalfWords(3, sprite.getColors(), paletteBase, sprite.getColorCount() * 2);
        vramSetBankE(VRAM_E_TEX_PALETTE);

        sprEntry->allocX = 8, sprEntry->allocY = 8;
        while (sprEntry->allocX < sprEntry->tileWidth * 8)
            sprEntry->allocX <<= 1;
        while (sprEntry->allocY < sprEntry->tileHeight * 8)
            sprEntry->allocY <<= 1;

        uint16_t neededTiles = sprEntry->allocX * sprEntry->allocY;
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
            return -3;
        }

        if (length == neededTiles) {
            nocashMessage("alloc change");
            // Remove free zone
            tileFreeZoneCount--;
            auto* newFreeZones = (uint16_t*) malloc(tileFreeZoneCount * 4);
            // Copy free zones up to freeZoneIdx
            memcpy(newFreeZones, tileFreeZones, freeZoneIdx * 4);
            // Copy free zones after freeZoneIdx
            memcpy((uint8_t*)newFreeZones + freeZoneIdx * 4,
                   (uint8_t*)tileFreeZones + (freeZoneIdx + 1) * 4,
                   (tileFreeZoneCount - freeZoneIdx) * 4);
            // Free old tileZones and change reference
            free(tileFreeZones);
            tileFreeZones = newFreeZones;
        }
        else {
            tileFreeZones[freeZoneIdx * 2] += neededTiles;
            tileFreeZones[freeZoneIdx * 2 + 1] -= neededTiles;
            sprintf(buffer, "reduce change idx %d start %d length %d",
                    freeZoneIdx, tileFreeZones[freeZoneIdx * 2], tileFreeZones[freeZoneIdx * 2 + 1]);
            nocashMessage(buffer);
        }

        sprEntry->tileStart = start;
        sprintf(buffer, "SPR tile start %d palette %d", start, sprEntry->paletteIdx);
        nocashMessage(buffer);
        int loadResult = loadSpriteFrame(sprEntry, 0);
        if (loadResult < 0) {
            free(sprEntry);
            res = nullptr;
            return loadResult - 3;
        }

        res = sprEntry;
        auto** activeSpriteNew = (SpriteManager**) malloc(sizeof(SpriteManager**) * (activeSpriteCount + 1));
        memcpy(activeSpriteNew, activeSprites, sizeof(SpriteManager**) * activeSpriteCount);
        activeSpriteNew[activeSpriteCount] = sprEntry;
        free(activeSprites);
        activeSprites = activeSpriteNew;

        auto* sprControl = (SpriteControl*) malloc(sizeof(SpriteControl));
        memset(sprControl, 0, sizeof(SpriteControl));

        auto** sprControlsNew = (SpriteControl**) malloc(sizeof(SpriteControl**) * (activeSpriteCount + 1));
        memcpy(sprControlsNew, activeSpriteControls, sizeof(SpriteControl**) * activeSpriteCount);
        sprControlsNew[activeSpriteCount] = sprControl;
        free(activeSpriteControls);
        activeSpriteControls = sprControlsNew;

        activeSpriteCount++;
        return 0;
    }

    void Sprite3DManager::freeSprite(Engine::SpriteManager *&spr) {
        if (spr == nullptr)
            return;
        int sprIdx = -1;
        for (int i = 0; i < activeSpriteCount; i++) {
            if (spr == activeSprites[i]) {
                sprIdx = i;
                break;
            }
        }
        if (sprIdx == -1)
            return;

        paletteUsed[spr->paletteIdx] = false;

        uint16_t start = spr->tileStart;
        uint16_t length = spr->allocX * spr->allocY;

        int freeAfterIdx = 0;
        for (; freeAfterIdx < tileFreeZoneCount; freeAfterIdx++) {
            if (tileFreeZones[freeAfterIdx * 2] > start) {
                break;
            }
        }

        bool mergePrev = false, mergePost = false;

        if (freeAfterIdx > 0)
            mergePrev = (tileFreeZones[(freeAfterIdx - 1) * 2] + tileFreeZones[freeAfterIdx * 2 -1]) == start;
        if (freeAfterIdx <= tileFreeZoneCount - 1) {
            char buffer[100];
            sprintf(buffer, "start %d len %d %d %d startfree %d",
                    start, length, spr->allocX, spr->allocY, tileFreeZones[freeAfterIdx * 2]);
            nocashMessage(buffer);
            mergePost = (start + length) == tileFreeZones[freeAfterIdx * 2];
        }

        if (mergePost && mergePrev)
        {
            tileFreeZoneCount--;
            auto* newFreeZones = (uint16_t*) malloc(4 * tileFreeZoneCount);
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[(freeAfterIdx - 1) * 2 + 1] += length + tileFreeZones[freeAfterIdx * 2 + 1];
            memcpy((uint8_t*)newFreeZones + freeAfterIdx * 4,
                   (uint8_t*)tileFreeZones + (freeAfterIdx + 1) * 4,
                   (tileFreeZoneCount - freeAfterIdx) * 4);
            free(tileFreeZones);
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
            auto* newFreeZones = (uint16_t*) malloc(4 * tileFreeZoneCount);
            memcpy(newFreeZones, tileFreeZones, freeAfterIdx * 4);
            newFreeZones[freeAfterIdx * 2] = start;
            newFreeZones[freeAfterIdx * 2 + 1] = start;
            memcpy((uint8_t*)newFreeZones + (freeAfterIdx + 1) * 4,
                   tileFreeZones + freeAfterIdx * 4,
                   (tileFreeZoneCount - (freeAfterIdx + 1)) * 4);
            free(tileFreeZones);
            tileFreeZones = newFreeZones;
        }

        SpriteControl* sprControl = activeSpriteControls[sprIdx];
        free(sprControl);
        sprControl = nullptr;

        auto** activeSpriteNew = (SpriteManager**) malloc(sizeof(SpriteManager**) * (activeSpriteCount - 1));
        memcpy(activeSpriteNew, activeSprites, sizeof(SpriteManager**) * sprIdx);
        memcpy(&activeSpriteNew[sprIdx], &activeSprites[sprIdx + 1],
               sizeof(SpriteManager**) * (activeSpriteCount - sprIdx - 1));
        free(activeSprites);
        activeSprites = activeSpriteNew;

        auto** sprControlsNew = (SpriteControl**) malloc(sizeof(SpriteControl**) * (activeSpriteCount + 1));
        memcpy(sprControlsNew, activeSpriteControls, sizeof(SpriteControl**) * sprIdx);
        memcpy(&sprControlsNew[sprIdx], &activeSpriteControls[sprIdx + 1],
               sizeof(SpriteManager**) * (activeSpriteCount - sprIdx - 1));
        free(activeSpriteControls);
        activeSpriteControls = sprControlsNew;

        activeSpriteCount--;

        free(spr);
        spr = nullptr;
    }

    int Sprite3DManager::loadSpriteFrame(Engine::SpriteManager *spr, int frame) {
        if (spr == nullptr)
            return -1;
        if (spr->currentFrame == frame)
            return -2;
        if (frame >= spr->frameCount || frame < 0)
            return -3;
        spr->currentFrame = frame;

        vramSetBankB(VRAM_B_LCD);

        uint8_t *tileRamStart = (uint8_t *) VRAM_B + spr->tileStart * 64;

        for (int y = 0; y < spr->tileHeight * 8; y++) {
            for (int x = 0; x < spr->tileWidth * 8; x++) {
                int tileX = x / 8;
                int tileY = y / 8;

                uint16_t framePos = frame * spr->tileWidth * spr->tileHeight;
                uint32_t tileOffset = framePos + tileY * spr->tileWidth + tileX;
                tileOffset *= 64;
                tileOffset += (y % 8) * 8 + (x % 8);
                *(uint16_t*)(tileRamStart + y * spr->allocX + x) &= ~(0xFF << (8 * (x & 1)));
                *(uint16_t*)(tileRamStart + y * spr->allocX + x) |= (spr->tileData[tileOffset] & 0xFF) << (8 * (x & 1));
            }
        }

        vramSetBankB(VRAM_B_TEXTURE_SLOT0);
        return 0;
    }

    void Sprite3DManager::draw() {
        for (int i = 0; i < activeSpriteCount; i++) {
            SpriteManager* spr = activeSprites[i];
            SpriteControl* activeControl = activeSpriteControls[i];

            if (spr->currentAnimation > 0) {
                CSPRAnimation* current = &spr->animations[spr->currentAnimation];
                if (current->frames[spr->currentAnimationFrame].duration != 0) {
                    spr->currentAnimationTimer--;
                    if (spr->currentAnimationTimer == 0) {
                        spr->currentAnimationFrame++;
                        spr->currentAnimationFrame %= current->frameCount;
                        loadSpriteFrame(spr, current->frames[spr->currentAnimationFrame].frame);
                        spr->currentAnimationTimer = current->frames[spr->currentAnimationFrame].duration;
                    }
                }
            }

            glColor( RGB15(31,31,31) );
            glPolyFmt( POLY_ALPHA(31) | POLY_CULL_NONE);

            uint8_t allocXFmt = 0;
            for (int x = spr->allocX; x > 8; x >>= 1) {
                allocXFmt += 1;
            }
            uint8_t allocYFmt = 0;
            for (int x = spr->allocY; x > 8; x >>= 1) {
                allocYFmt += 1;
            }

            MATRIX_CONTROL = GL_MODELVIEW;
            MATRIX_IDENTITY = 0;
            uint32_t x = activeControl->x;
            uint32_t x2 = activeControl->x + spr->tileWidth * 8;
            uint32_t w = spr->tileWidth * 8;
            uint32_t y = activeControl->y;
            uint32_t y2 = activeControl->y + spr->tileHeight * 8;
            uint32_t h = spr->tileHeight * 8;
            GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) + (4 << 26) + (1 << 29) + spr->tileStart * 8;
            GFX_PAL_FORMAT = spr->paletteIdx * 2 * 256 / 16;
            GFX_BEGIN = GL_QUADS;
            GFX_TEX_COORD = 0;
            GFX_VERTEX16 = x + (y << 16);
            GFX_VERTEX16 = activeControl->layer;
            GFX_TEX_COORD = h << (4 + 16);
            GFX_VERTEX_XY = x + (y2 << 16);
            GFX_TEX_COORD = (h << (4 + 16)) + (w << 4);
            GFX_VERTEX_XY = x2 + (y2 << 16);
            GFX_TEX_COORD = (w << 4);
            GFX_VERTEX_XY = x2 + (y << 16);
            GFX_END = 0;
        }
    }

    SpriteControl* Sprite3DManager::getSpriteControl(SpriteManager* manager) {
        if (manager == nullptr)
            return nullptr;
        for (int i = 0; i < activeSpriteCount; i++) {
            if (activeSprites[i] == manager) {
                return activeSpriteControls[i];
            }
        }
        return nullptr;
    }

    void Sprite3DManager::setSpriteAnim(Engine::SpriteManager *spr, int animId) {
        if (spr == nullptr)
            return;
        if (animId >= spr->animCount)
            return;
        if (spr->currentAnimation == animId)
            return;
        spr->currentAnimation = animId;
        spr->currentAnimationFrame = 0;
        spr->currentAnimationTimer = spr->animations[animId].frames[0].duration;
        loadSpriteFrame(spr, spr->animations[animId].frames[0].frame);
    }

    Sprite3DManager main3dSpr;
}
