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
#include "Engine/math.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/OAMManager.hpp"
#include "TitleScreen.hpp"
#include "WriteName.hpp"
#include "Font.hpp"
#include "Room.hpp"
#include "Player.hpp"
#include "Camera.hpp"

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

    Engine::textMain.clear();
    Engine::textSub.clear();

    globalPlayer = new Player();
    globalPlayer->showPlayer();
    globalPlayer->spriteManager.wx = 144 << 8;
    globalPlayer->spriteManager.wy = 121 << 8;
    globalRoom = new Room(0);
    globalCamera.updatePosition(true);

    for (;;) {
        Engine::tick();
        if (keysDown() & KEY_START)
            break;
        globalPlayer->update();
        globalCamera.updatePosition(false);
        globalPlayer->draw();
        globalRoom->draw();
    }

    return 0;
}