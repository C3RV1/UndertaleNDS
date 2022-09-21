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
#include "Engine/Texture.hpp"
#include "Engine/OAMManager.hpp"
#include "TitleScreen.hpp"
#include "WriteName.hpp"
#include "Font.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"

int main() {
    /* Configure the VRAM and background control registers. */
    if (Engine::init() != 0)
        return 0;

    runTitleScreen();
    writeNameMenu();

    for (int i = 0; i < 5; i++) {
        globalSave.items[i] = 1 + (i % 3 == 0);
    }

    Engine::textMain.clear();
    Engine::textSub.clear();

    uint16_t roomSpawn = 0;

    // DEBUG
    // roomSpawn = 2;
    // globalSave.flags[0] = 1;

    globalPlayer = new Player();
    globalPlayer->spriteManager.setShown(true);
    globalInGameMenu.load();
    globalInGameMenu.show();
    globalRoom = new Room(roomSpawn);
    globalCamera.updatePosition(true);
    globalPlayer->spriteManager.wx = globalRoom->spawnX << 8;
    globalPlayer->spriteManager.wy = globalRoom->spawnY << 8;

    for (;;) {
        Engine::tick();
        globalPlayer->update();
        globalRoom->update();
        if (globalCutscene != nullptr) {
            if (currentDialogue != nullptr) {
                if (currentDialogue->update()) {
                    currentDialogue->free_();
                    delete currentDialogue;
                    currentDialogue = nullptr;
                }
            }
            if (globalCutscene->runCommands(ROOM)) {
                delete globalCutscene;
                globalCutscene = nullptr;
                globalInGameMenu.show();
                globalPlayer->playerControl = true;
                globalCamera.manual = false;
            }
        }
        globalCamera.updatePosition(false);
        globalInGameMenu.update();
        globalPlayer->draw();
        globalRoom->draw();
    }

    return 0;
}