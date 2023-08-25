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

        // OAM
        u8 oamScaleIdx = 0xff;  // all oam entries can share scale
        std::vector<u8> oamEntries;
        int palette = -1;

        int loadedFrame = -1;
        bool loadedIntoMemory = false;
    };

    class Sprite {
    public:
        explicit Sprite(AllocationMode allocMode=NoAlloc);
        void setAllocationMode(AllocationMode allocMode);
        void setSpriteAnim(int animId);
        void setFrame(int frameId);
        void loadTexture(Texture& texture);
        int nameToAnimId(const std::string& animName) const;
        void tick();
        void setShown(bool shown);
        void push();
        void pop();
        ~Sprite() { setShown(false); }

        bool _loaded = false;
        Texture* _texture = nullptr;

        s32 _wx = 0, _wy = 0;  // 1 bit sign, 23 bit integer, 8 bit fraction, world
        s32 _w_scale_x = 1 << 8, _w_scale_y = 1 << 8;
        s32 _cam_x = 0, _cam_y = 0;
        s32 _cam_scale_x = 1 << 8, _cam_scale_y = 1 << 8;
        s32 _layer = 0;
        int _cAnimation = -1;

        friend class OAMManager;
        friend class Sprite3DManager;
    private:
        s32 _x = 0, _y = 0;  // 1 bit sign, 23 bit integer, 8 bit fraction, screen
        s32 _scale_x = 0, _scale_y = 0;
        u16 _cAnimTimer = 0;
        u16 _cAnimFrame = 0;
        int _cFrame = 0;
        SpriteInternalMemory _memory;
        AllocationMode _allocMode;
        bool _shown = false;
        bool _pushed = false;
    };
}

#endif //UNDERTALE_SPRITE_HPP
