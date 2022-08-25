//
// Created by cervi on 20/08/2022.
//

#ifndef LAYTON_OAMMANAGER_HPP
#define LAYTON_OAMMANAGER_HPP

#define ARM9
#include <nds.h>
#include "Sprite.hpp"

namespace Engine {
    struct SpriteManager {
        uint8_t frameCount = 0;
        uint8_t oamEntryCount = 0;
        union {
            uint8_t * oamEntries = nullptr;
            uint16_t tileStart;
        };
        uint8_t* tileData = nullptr;
        uint8_t tileWidth = 0, tileHeight = 0;
        uint16_t allocX = 0, allocY = 0;
        uint8_t colorCount = 0;
        union {
            uint8_t *paletteColors = nullptr;
            uint8_t paletteIdx;
        };
    };

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
            tileFreeZones = (uint16_t*) malloc(4);
            tileFreeZones[0] = 1;
            tileFreeZones[1] = 1023;
            *paletteRam = 31 << 5;  // full green for bg
        };

        int loadSprite(Sprite& sprite, SpriteManager*& res);
        void freeSprite(SpriteManager*& spr);
        void setSpritePos(SpriteManager* spr, int x, int y);
        int loadSpriteFrame(SpriteManager* spr, int frame);

        int reserveOAMEntry(uint8_t tileW, uint8_t tileH);
        void freeOAMEntry(int oamId);
    private:
        u16* paletteRam;
        u16* oamRam;
        u16* tileRam;

        uint16_t tileFreeZoneCount;
        uint16_t* tileFreeZones = nullptr;  // pairs of (starting addr, free tiles)

        uint8_t paletteRefCounts[255] = {0};
        OAMEntry oamEntries[SPRITE_COUNT];
    };

    extern OAMManager OAMSub;
}

#endif //LAYTON_OAMMANAGER_HPP
