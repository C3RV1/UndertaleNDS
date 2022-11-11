//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_SPRITE_3D_MANAGER_HPP
#define UNDERTALE_SPRITE_3D_MANAGER_HPP

#include "Sprite.hpp"
#include "Engine/FreeZoneManager.hpp"
#define ARM9
#include <nds.h>

namespace Engine {
    class Sprite3DManager {
    public:
        Sprite3DManager() :
            tileFreeZones(0, 65536 - 8, "3D_TILES"),
            paletteFreeZones(0, 1024, "3D_PALETTE") {}

        void draw();
        void updateTextures();
    private:
        friend class Sprite;

        int loadSprite(Sprite& res);
        void freeSprite(Sprite& spr);

        void loadSpriteTexture(Sprite& spr);
        void freeSpriteTexture(Sprite& spr);

        FreeZoneManager tileFreeZones;
        FreeZoneManager paletteFreeZones;

        bool _paletteUsed[96] = {false};

        u8 _activeSprCount = 0;
        Sprite** _activeSpr = nullptr;
    };

    extern Sprite3DManager main3dSpr;
}

#endif //UNDERTALE_SPRITE_3D_MANAGER_HPP
