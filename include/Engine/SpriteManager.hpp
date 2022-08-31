//
// Created by cervi on 28/08/2022.
//

#ifndef LAYTON_SPRITEMANAGER_HPP
#define LAYTON_SPRITEMANAGER_HPP

namespace Engine {
    class SpriteManager;
}

#define ARM9
#include <nds.h>
#include "Sprite.hpp"
#include "OAMManager.hpp"
#include "Sprite3DManager.hpp"

namespace Engine {
    enum AllocationMode {
        NoAlloc = 0,
        AllocatedOAM = 1,
        Allocated3D = 2
    };

    struct SpriteInternalMemory {
        AllocationMode allocated = NoAlloc;
        uint8_t oamEntryCount = 0;
        uint8_t oamScaleIdx = 0xff;  // all oam entries can share scale
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
        explicit SpriteManager(AllocationMode allocMode_);
        void setSpriteAnim(int animId);
        void loadSprite(Sprite& sprite_);
        int nameToAnimId(const char *animName) const;
        void tick();
        void setShown(bool shown_);
        void push();
        void pop();

        bool loaded = false;
        Sprite* sprite = nullptr;

        int32_t x = 0, y = 0;  // 23 bit integer, 8 bit fraction, screen
        int32_t wx = 0, wy = 0;  // 23 bit integer, 8 bit fraction, world
        int32_t wscale_x = 1 << 8, wscale_y = 1 << 8;
        int32_t scale_x = 0, scale_y = 0;
        int32_t cam_x = 0, cam_y = 0;
        int32_t cam_scale_x = 1 << 8, cam_scale_y = 1 << 8;
        int32_t layer = 0;
        int currentFrame = -1;
        int currentAnimation = -1;
        uint16_t currentAnimationTimer = 0;
        uint16_t currentAnimationFrame = 0;

        SpriteInternalMemory memory;
    private:
        AllocationMode allocMode;
        bool shown = false;
        bool pushed = false;
    };
}

#endif //LAYTON_SPRITEMANAGER_HPP
