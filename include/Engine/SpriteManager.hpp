//
// Created by cervi on 28/08/2022.
//

#ifndef LAYTON_SPRITEMANAGER_HPP
#define LAYTON_SPRITEMANAGER_HPP

#define ARM9
#include <nds.h>
#include "Sprite.hpp"

namespace Engine {
    enum AllocationMode {
        NoAlloc = 0,
        AllocatedOAM = 1,
        Allocated3D = 2
    };

    struct SpriteInternalMemory {
        AllocationMode allocated = NoAlloc;
        uint8_t oamEntryCount = 0;
        union {
            uint8_t * oamEntries = nullptr;
            uint16_t tileStart;
        };
        uint16_t allocX = 0, allocY = 0;
        union {
            uint8_t *paletteColors = nullptr;
            uint8_t paletteIdx;
        };
        int loadedFrame = -1;
    };

    class SpriteManager {
    public:
        void setSpriteAnim(int animId);
        void loadSprite(Sprite& sprite_);
        void copyNoMemory(SpriteManager& spriteManager);

        bool loaded = false;
        Sprite* sprite = nullptr;

        int32_t x = 0, y = 0;  // 23 bit integer, 8 bit fraction, screen
        int32_t wx = 0, wy = 0;  // 23 bit integer, 8 bit fraction, world
        int32_t layer = 0;
        int currentFrame = -1;
        int currentAnimation = -1;
        uint16_t currentAnimationTimer = 0;
        uint16_t currentAnimationFrame = 0;

        SpriteInternalMemory memory;
    };
}

#endif //LAYTON_SPRITEMANAGER_HPP
