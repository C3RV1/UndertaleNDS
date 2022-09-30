//
// Created by cervi on 30/09/2022.
//
#include "MainMenu.hpp"
#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Engine.hpp"
#include "Save.hpp"
#include "Formats/utils.hpp"
#include <stdio.h>

void runMainMenu() {
    const int nameX = 42, nameY = 24 - 4;
    const int lvX = 132, lvY = 24 - 4;
    const int roomNameX = 42, roomNameY = 44 - 4;
    char buffer[100];
    char *roomName = nullptr;
    char *continueText = nullptr;
    char *resetText = nullptr;

    Engine::Background topBg;
    Engine::Background btmBg;
    Engine::Font font;

    topBg.loadPath("main_menu_top");
    btmBg.loadPath("main_menu_btm");
    font.loadPath("fnt_maintext.font");

    sprintf(buffer, "nitro:/data/rooms/names/%d.txt", globalSave.lastSavedRoom);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        int len = strlen_file(f, '\n');
        roomName = new char[len + 1];
        fread(roomName, len, 1, f);
        roomName[len] = 0;
    } else {
        sprintf(buffer, "Error opening room %d name", globalSave.lastSavedRoom);
        nocashMessage(buffer);
    }
    fclose(f);

    f = fopen("nitro:/data/main_menu.txt", "rb");
    if (f) {
        int len = strlen_file(f, '\n');
        continueText = new char[len + 1];
        fread(continueText, len, 1, f);
        continueText[len] = 0;

        len = strlen_file(f, '\n');
        resetText = new char[len + 1];
        fread(resetText, len, 1, f);
        resetText[len] = 0;
    } else {
        sprintf(buffer, "Error opening room %d name", globalSave.lastSavedRoom);
        nocashMessage(buffer);
    }
    fclose(f);

    Engine::loadBgTextMain(topBg);
    Engine::loadBgTextSub(btmBg);
    Engine::textSub.clear();

    int x = nameX;
    for (char* p = globalSave.name; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, nameY);
    }

    sprintf(buffer, "%d", globalSave.lv);
    x = lvX;
    for (char* p = buffer; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, lvY);
    }

    if (roomName != nullptr) {
        x = roomNameX;
        for (char* p = roomName; *p != 0; roomName++) {
            Engine::textSub.drawGlyph(font, *p, x, roomNameY);
        }
    }

    bool draw = true;
    bool running = true;
    int selected = 0;
    while (running) {
        Engine::tick();
        if ((keysDown() & KEY_L) && (selected == 0)) {
            selected = 1;
            draw = true;
        } else if ((keysDown() & KEY_R) && (selected == 1)) {
            selected = 0;
            draw = true;
        }

        if (draw) {

        }
    }

    delete[] roomName;
    delete[] continueText;
    delete[] resetText;
}
