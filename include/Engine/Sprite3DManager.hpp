//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_SPRITE3DMANAGER_HPP
#define UNDERTALE_SPRITE3DMANAGER_HPP

#include "Sprite.hpp"
#define ARM9
#include <nds.h>

namespace Engine {
    class Sprite3DManager {
    public:
        Sprite3DManager() {
            tileFreeZoneCount = 1;
            tileFreeZones = new u16[2];
            tileFreeZones[0] = 0;
            tileFreeZones[1] = 65536-8;
        }

        int loadSprite(Sprite& res);
        void freeSprite(Sprite& spr);

        void draw();
        void updateTextures();
    private:
        static void loadSpriteTexture(Sprite& spr);
        static void loadPalette(Sprite& spr);
        int reserveTiles(u16 length, u16& start);
        void freeTiles(u16 length, u16& start);

        u16 tileFreeZoneCount;
        u16* tileFreeZones;

        bool paletteUsed[96] = {false};

        u8 activeSpriteCount = 0;
        Sprite** activeSprites = nullptr;
    };

    extern Sprite3DManager main3dSpr;
}

#endif //UNDERTALE_SPRITE3DMANAGER_HPP
