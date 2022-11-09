//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_SPRITE_3D_MANAGER_HPP
#define UNDERTALE_SPRITE_3D_MANAGER_HPP

#include "Sprite.hpp"
#define ARM9
#include <nds.h>

namespace Engine {
    class Sprite3DManager {
    public:
        Sprite3DManager() {
            _tileFreeZoneCount = 1;
            _tileFreeZones = new u16[2];
            _tileFreeZones[0] = 0;
            _tileFreeZones[1] = 65536 - 8;
        }

        void draw();
        void updateTextures();
    private:
        friend class Sprite;

        int loadSprite(Sprite& res);
        void freeSprite(Sprite& spr);

        void loadSpriteTexture(Sprite& spr);
        void freeSpriteTexture(Sprite& spr);
        int reserveTiles(u16 length, u16& start);
        void freeTiles(u16 length, u16& start);

        u16 _tileFreeZoneCount;
        u16* _tileFreeZones;

        bool _paletteUsed[96] = {false};

        u8 _activeSprCount = 0;
        Sprite** _activeSpr = nullptr;
    };

    extern Sprite3DManager main3dSpr;
}

#endif //UNDERTALE_SPRITE_3D_MANAGER_HPP
