/*
 *  main.cpp
 *
 *  Created by Jaeden Amero on 11/12/07.
 *  Copyright 2007. All rights reserved.
 *
 */

#define ARM9
#include <nds.h>
#include <filesystem.h>
#include <stdio.h>
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/OAMManager.hpp"
#include "TitleScreen.hpp"
#include "WriteName.hpp"
#include "Font.hpp"

int main() {
    /* Configure the VRAM and background control registers. */
    if (Engine::init() != 0)
        return 0;

    for (;;) {
        Engine::tick();
        if (keysDown() & KEY_START)
            break;
    }

    runTitleScreen();
    writeNameMenu();

    for (;;) {
        Engine::tick();
        if (keysDown() & KEY_START)
            break;
    }

    return 0;
}