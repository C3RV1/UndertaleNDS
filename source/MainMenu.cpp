//
// Created by cervi on 30/09/2022.
//
#include "MainMenu.hpp"
#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Audio.hpp"
#include "Save.hpp"
#include "Formats/utils.hpp"
#include <cstdio>

void runMainMenu() {
    const int nameX = 42, nameY = 24 - 4;
    const int lvX = 132, lvY = 24 - 4;
    const int roomNameX = 42, roomNameY = 44 - 4;
    const int continueX = 63, continueY = 67 - 4;
    const int resetX = 156, resetY = 67 - 4;
    char buffer[100];
    char *roomName = nullptr;
    char *continueText = nullptr;
    char *resetText = nullptr;

    Engine::Background topBg;
    Engine::Background btmBg;
    Engine::Texture floweyTexture;
    Engine::Sprite floweySprite(Engine::AllocatedOAM);
    Engine::Font font;

    topBg.loadPath("main_menu_top");
    btmBg.loadPath("main_menu_btm");
    font.loadPath("fnt_maintext.font");

    if (globalSave.flags[0] < 20) {
        floweyTexture.loadPath("room_sprites/flowey");
        floweySprite.loadTexture(floweyTexture);
        floweySprite.wx = 118 << 8; floweySprite.wy = 116 << 8;
        floweySprite.setShown(true);
        Audio::playBGMusic("mus_menu1.wav", true);
    }

    sprintf(buffer, "nitro:/data/room_names/%d.txt", globalSave.lastSavedRoom);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        int len = str_len_file(f, '\n');
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
        int len = str_len_file(f, '\n');
        continueText = new char[len + 1];
        fread(continueText, len + 1, 1, f);
        continueText[len] = 0;

        len = str_len_file(f, '\n');
        resetText = new char[len + 1];
        fread(resetText, len + 1, 1, f);
        resetText[len] = 0;
    } else {
        sprintf(buffer, "Error opening room %d name", globalSave.lastSavedRoom);
        nocashMessage(buffer);
    }
    fclose(f);

    topBg.loadBgTextMain();
    btmBg.loadBgTextSub();
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
        for (char* p = roomName; *p != 0; p++) {
            Engine::textSub.drawGlyph(font, *p, x, roomNameY);
        }
    }

    bool draw = true;
    int selected = 0;
    bool resetConfirm = false;
    while (true) {
        Engine::tick();
        if ((keysDown() & KEY_RIGHT) && (selected == 0)) {
            selected = 1;
            draw = true;
        } else if ((keysDown() & KEY_LEFT) && (selected == 1)) {
            selected = 0;
            draw = true;
            resetConfirm = false;
        }

        if (keysDown() & KEY_A) {
            if (selected == 1 && !resetConfirm) {
                resetConfirm = true;
                draw = true;
            }
            else if (resetConfirm) {
                globalSave.clear(PLAYER_RESET);
                break;
            } else {
                break;
            }
        }

        if (draw && continueText != nullptr && resetText != nullptr) {
            if (selected == 0)
                Engine::textSub.setCurrentColor(12);
            else
                Engine::textSub.setCurrentColor(15);
            x = continueX;
            for (char *p = continueText; *p != 0; p++) {
                Engine::textSub.drawGlyph(font, *p, x, continueY);
            }

            if (selected == 1 && !resetConfirm)
                Engine::textSub.setCurrentColor(12);
            else if (resetConfirm)
                Engine::textSub.setCurrentColor(9);
            else
                Engine::textSub.setCurrentColor(15);
            x = resetX;
            for (char *p = resetText; *p != 0; p++) {
                Engine::textSub.drawGlyph(font, *p, x, resetY);
            }

            draw = false;
        }
    }

    Audio::stopBGMusic();
    topBg.free_();
    btmBg.free_();
    floweySprite.setShown(false);
    floweyTexture.free_();
    font.free_();

    delete[] roomName;
    delete[] continueText;
    delete[] resetText;
}
