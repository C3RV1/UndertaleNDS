//
// Created by cervi on 20/08/2022.
//

#ifndef UNDERTALE_OAM_MANAGER_HPP
#define UNDERTALE_OAM_MANAGER_HPP

#include <nds.h>
#include <memory>
#include <vector>
#include "Engine/FreeZoneManager.hpp"
#include "Sprite.hpp"
#include "DEBUG_FLAGS.hpp"

namespace Engine {
    struct OAMEntry {
        bool free_ = true;
        u16 tileStart = 0;
        u8 tileWidth = 0, tileHeight = 0;
    };

    class OAMManager {
    public:
        OAMManager(u16* paletteRam,
                   u16* tileRam,
                   u16* oamRam);

#ifdef DEBUG_2D
        void dumpOamState();
#endif

        void draw();
    private:
        friend class Sprite;

        int loadSprite(Sprite& res);
        void freeSprite(Sprite& spr);

        int reserveOAMEntry(u8 tileW, u8 tileH);
        void freeOAMEntry(int oamId);

        void setSpritePosAndScale(Sprite& spr);
        int loadSpriteFrame(Sprite& spr, int frame);
        void setOAMState(Sprite& spr);
        void allocateOamScaleEntry(Sprite& spr);
        void freeOamScaleEntry(Sprite& spr);

        u16* _paletteRam;
        u16* _oamRam;
        u16* _tileRam;

        FreeZoneManager _tileZones;
        std::vector<Sprite*> _activeSpr;

        u8 _paletteRefCounts[16] = {0};
        OAMEntry _oamEntries[SPRITE_COUNT];
        bool _oamScaleEntryUsed[32] = {false};
    };

    extern OAMManager OAMManagerSub;
}

#endif //UNDERTALE_OAM_MANAGER_HPP
