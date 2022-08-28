//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_SPRITE3DMANAGER_HPP
#define LAYTON_SPRITE3DMANAGER_HPP

#include "Sprite.hpp"
#include "SpriteManager.hpp"
#define ARM9
#include <nds.h>

namespace Engine {
    struct SpriteControl {
        int x = 0, y = 0;
        int layer = 0;
    };

    class Sprite3DManager {
    public:
        Sprite3DManager() {
            tileFreeZoneCount = 1;
            tileFreeZones = (uint16_t*) malloc(4);
            tileFreeZones[0] = 1;
            tileFreeZones[1] = 65535;
        }

        int loadSprite(Sprite& sprite, SpriteManager*& res);
        void freeSprite(SpriteManager*& spr);
        int loadSpriteFrame(SpriteManager* spr, int frame);
        void setSpriteAnim(SpriteManager* spr, int animId);
        SpriteControl* getSpriteControl(SpriteManager* manager);

        void draw();
    private:
        uint16_t tileFreeZoneCount;
        uint16_t* tileFreeZones = nullptr;

        bool paletteUsed[96] = {false};

        uint8_t activeSpriteCount = 0;
        SpriteManager** activeSprites = nullptr;
        SpriteControl** activeSpriteControls = nullptr;
    };

    extern Sprite3DManager main3dSpr;
}

#endif //LAYTON_SPRITE3DMANAGER_HPP
