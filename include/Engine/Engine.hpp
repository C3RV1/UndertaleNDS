#ifndef LAYTON_ENGINE_HPP
#define LAYTON_ENGINE_HPP

#define ARM9
#include <nds.h>
#include <filesystem.h>
#include "Background.hpp"
#include "Font.hpp"
#include "BGM.hpp"
#include <maxmod9.h>

namespace Engine {
    int init();
    void tick();
    int loadBgMain(Background& f);
    int loadBgSub(Background& f);
    int loadBgEngine(Background& bg, vu16* bg3Reg, u16* paletteRam,
                     u16* tileRam, u16* mapRam);
}

#endif