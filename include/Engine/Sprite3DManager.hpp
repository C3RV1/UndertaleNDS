//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_SPRITE3DMANAGER_HPP
#define LAYTON_SPRITE3DMANAGER_HPP

namespace Engine {
    class Sprite3DManager;
    extern Sprite3DManager main3dSpr;
}

#include "Sprite.hpp"
#include "SpriteManager.hpp"
#define ARM9
#include <nds.h>

namespace Engine {
    class Sprite3DManager {
    public:
        Sprite3DManager() {
            tileFreeZoneCount = 1;
            tileFreeZones = new uint16_t[2];
            tileFreeZones[0] = 1;
            tileFreeZones[1] = 65535;
        }

        int loadSprite(SpriteManager& res);
        void freeSprite(SpriteManager& spr);

        void draw();
    private:
        int loadSpriteFrame(SpriteManager& spr, int frame);

        uint16_t tileFreeZoneCount;
        uint16_t* tileFreeZones = nullptr;

        bool paletteUsed[96] = {false};

        uint8_t activeSpriteCount = 0;
        SpriteManager** activeSprites = nullptr;
    };
}

#endif //LAYTON_SPRITE3DMANAGER_HPP
