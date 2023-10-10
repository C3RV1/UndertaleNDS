//
// Created by cervi on 30/09/2022.
//
#include "MainMenu.hpp"
#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Texture.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/WAV.hpp"
#include "Save.hpp"
#include "Formats/utils.hpp"
#include <cstdio>

void runMainMenu() {
    constexpr int nameX = 42, nameY = 24 - 4;
    constexpr int lvX = 132, lvY = 24 - 4;
    constexpr int roomNameX = 42, roomNameY = 44 - 4;
    constexpr int continueX = 63, continueY = 67 - 4;
    constexpr int resetX = 156, resetY = 67 - 4;
    std::string buffer;
    std::string roomName;
    std::string continueText;
    std::string resetText;

    Engine::Background topBg;
    Engine::Background btmBg;
    Engine::Texture floweyTex;
    Engine::Sprite floweySpr(Engine::AllocatedOAM);
    Engine::Font font;

    topBg.loadPath("main_menu_top");
    btmBg.loadPath("main_menu_btm");
    font.loadPath("fnt_maintext.font");

    if (globalSave.flags[FlagIds::PROGRESS] < 20) {
        floweyTex.loadPath("room_sprites/flowey");
        floweySpr.loadTexture(floweyTex);
        floweySpr._wx = 118 << 8; floweySpr._wy = 116 << 8;
        floweySpr.setShown(true);
        Audio2::playBGMusic("mus_menu1.wav", true);
    }

    buffer = "nitro:/data/room_names/" + std::to_string(globalSave.lastSavedRoom) + ".txt";
    FILE* f = fopen(buffer.c_str(), "rb");
    if (f) {
        int len = str_len_file(f, '\n');
        roomName.resize(len);
        fread(&roomName[0], len, 1, f);
        fseek(f, 1, SEEK_CUR);
    } else {
        buffer = "Error opening room " + std::to_string(globalSave.lastSavedRoom) + " name";
        nocashMessage(buffer.c_str());
    }
    fclose(f);

    f = fopen("nitro:/data/main_menu.txt", "rb");
    if (f) {
        int len = str_len_file(f, '\n');
        continueText.resize(len);
        fread(&continueText[0], len, 1, f);
        fseek(f, 1, SEEK_CUR);

        len = str_len_file(f, '\n');
        resetText.resize(len);
        fread(&resetText[0], len, 1, f);
        fseek(f, 1, SEEK_CUR);
    } else {
        nocashMessage("Error opening main menu text");
    }
    fclose(f);

    topBg.loadBgTextMain();
    btmBg.loadBgTextSub();
    Engine::textSub.clear();

    int x = nameX;
    for (char* p = globalSave.name; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, nameY);
    }

    buffer = std::to_string(globalSave.lv);
    x = lvX;
    for (auto const & c : buffer) {
        Engine::textSub.drawGlyph(font, c, x, lvY);
    }

    if (!roomName.empty()) {
        x = roomNameX;
        for (auto const & c : roomName) {
            Engine::textSub.drawGlyph(font, c, x, roomNameY);
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

        if (draw && !continueText.empty() && !resetText.empty()) {
            if (selected == 0)
                Engine::textSub.setColor(12);
            else
                Engine::textSub.setColor(15);
            x = continueX;
            for (auto const & c : continueText) {
                Engine::textSub.drawGlyph(font, c, x, continueY);
            }

            if (selected == 1 && !resetConfirm)
                Engine::textSub.setColor(12);
            else if (resetConfirm)
                Engine::textSub.setColor(9);
            else
                Engine::textSub.setColor(15);
            x = resetX;
            for (auto const & c : resetText) {
                Engine::textSub.drawGlyph(font, c, x, resetY);
            }

            draw = false;
        }
    }

    Audio2::stopBGMusic();
    floweySpr.setShown(false);
}
