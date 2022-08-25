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

    FILE* f = fopen("/bg/main1.cbgf", "rb");
    Engine::Background bg;
    if (f) {
        bg.loadCBGF(f);
    }
    fclose(f);

    f = fopen("spr/spr_f_mainchara.cspr", "rb");
    Engine::Sprite spr;
    if (f) {
        spr.loadCSPR(f);
    }
    fclose(f);

    Engine::textMain.clear();
    Engine::loadBgMain(bg);

    Engine::SpriteManager* sprManager;
    Engine::main3dSpr.loadSprite(spr, sprManager);
    int frame = 0;

    for (;;) {
        Engine::tick();
        if (keysDown() & KEY_START)
            break;
        Engine::main3dSpr.loadSpriteFrame(sprManager, frame / 60);
        frame++;
        frame = frame % (12 * 60);
    }

    return 0;
}