//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_SPRITE_HPP
#define UNDERTALE_SPRITE_HPP

namespace Engine {
    class Sprite;
}

#define ARM9
#include <nds.h>
#include "Engine/Texture.hpp"
#include "Engine/OAMManager.hpp"
#include "Engine/Sprite3DManager.hpp"

namespace Engine {
    enum AllocationMode {
        NoAlloc = 0,
        AllocatedOAM = 1,
        Allocated3D = 2
    };

    struct SpriteInternalMemory {
        AllocationMode allocated = NoAlloc;
        u8 oamEntryCount = 0;
        u8 oamScaleIdx = 0xff;  // all oam entries can share scale
        union {
            u8 * oamEntries = nullptr;
            u16 tileStart;
        };
        u16 allocX = 0, allocY = 0;
        union {
            u8 *paletteColors = nullptr;
            u8 paletteIdx;
        };
        int loadedFrame = -1;
        bool loadedOAM = false;
        bool loadedPalette = false;
    };

    class Sprite {
    public:
        explicit Sprite(AllocationMode allocMode_);
        void setSpriteAnim(int animId);
        void loadTexture(Texture& sprite_);
        int nameToAnimId(const char *animName) const;
        void tick();
        void setShown(bool shown_);
        void push();
        void pop();
        ~Sprite() { setShown(false); }

        bool loaded = false;
        Texture* texture = nullptr;

        s32 wx = 0, wy = 0;  // 1 bit sign, 22 bit integer, 8 bit fraction, world
        s32 wscale_x = 1 << 8, wscale_y = 1 << 8;
        s32 cam_x = 0, cam_y = 0;
        s32 cam_scale_x = 1 << 8, cam_scale_y = 1 << 8;
        s32 layer = 0;
        int currentFrame = 0;
        int currentAnimation = -1;

        friend class OAMManager;
        friend class Sprite3DManager;
    private:
        s32 x = 0, y = 0;  // 1 bit sign, 22 bit integer, 8 bit fraction, screen
        s32 scale_x = 0, scale_y = 0;
        u16 currentAnimationTimer = 0;
        u16 currentAnimationFrame = 0;
        SpriteInternalMemory memory;
        AllocationMode allocMode;
        bool shown = false;
        bool pushed = false;
    };
}

#endif //UNDERTALE_SPRITE_HPP
