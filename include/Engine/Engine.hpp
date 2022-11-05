#ifndef UNDERTALE_ENGINE_HPP
#define UNDERTALE_ENGINE_HPP

#define ARM9
#include <nds.h>
#include <fat.h>
#include <ctime>
#include "Engine/Background.hpp"
#include <maxmod9.h>

namespace Engine {
    int init();
    void tick();
}

#endif