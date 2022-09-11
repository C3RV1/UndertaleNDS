//
// Created by cervi on 20/08/2022.
//

#ifndef UNDERTALE_OAMMANAGER_HPP
#define UNDERTALE_OAMMANAGER_HPP

#define ARM9
#include <nds.h>
#include "Texture.hpp"
#include "Sprite.hpp"

namespace Engine {
    struct OAMEntry {
        bool free_ = true;
        uint16_t tileStart = 0;
        uint8_t tileWidth = 0, tileHeight = 0;
    };

    class OAMManager {
    public:
        OAMManager(u16* paletteRam,
                   u16* tileRam,
                   u16* oamRam) :
                paletteRam(paletteRam),
                oamRam(oamRam),
                tileRam(tileRam) {
            tileFreeZoneCount = 1;
            tileFreeZones = new uint16_t[2];
            tileFreeZones[0] = 1;
            tileFreeZones[1] = 1023;
            *paletteRam = 31 << 5;  // full green for bg
        };

        int loadSprite(Sprite& res);
        void freeSprite(Sprite& spr);

        int reserveOAMEntry(uint8_t tileW, uint8_t tileH);
        void freeOAMEntry(int oamId);

        void dumpOamState();

        void draw();
    private:
        void setSpritePosAndScale(Sprite& spr);
        int loadSpriteFrame(Sprite& spr, int frame);
        void allocateOamScaleEntry(Sprite& spr);
        void freeOamScaleEntry(Sprite& spr);

        u16* paletteRam;
        u16* oamRam;
        u16* tileRam;

        uint16_t tileFreeZoneCount;
        uint16_t* tileFreeZones;  // pairs of (starting addr, free tiles)

        uint8_t activeSpriteCount = 0;
        Sprite** activeSprites = nullptr;

        uint8_t paletteRefCounts[255] = {0};
        OAMEntry oamEntries[SPRITE_COUNT];
        bool oamScaleEntryUsed[32] = {false};
    };

    extern OAMManager OAMManagerSub;
}

#endif //UNDERTALE_OAMMANAGER_HPP
