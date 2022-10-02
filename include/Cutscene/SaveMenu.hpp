//
// Created by cervi on 28/09/2022.
//

#ifndef UNDERTALE_SAVE_MENU_HPP
#define UNDERTALE_SAVE_MENU_HPP

#define ARM9
#include <nds.h>
#include "Engine/Audio.hpp"
#include "Engine/Background.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Font.hpp"
#include "Save.hpp"

class SaveMenu {
public:
    SaveMenu();
    void drawInfo(SaveData& saveData, u8 color);
    bool update();
    void free_();
private:
    const int nameX = 41, nameY = 43 - 4;
    const int lvNumX = 131, lvNumY = 43 - 4;
    const int roomNameX = 41, roomNameY = 63 - 4;
    const int hrtSaveX = 54, hrtSaveY = 99;
    const int hrtRetX = 148, hrtRetY = 99;
    const int holdSaveFrames = 60;
    int currentHoldFrames = -1;
    int selectedOption = 0;
    char* roomName = nullptr;

    Audio::WAV saveSnd;
    Engine::Texture optionsHeartTex;
    Engine::Sprite optionsHeart;
    Engine::Font font;
    Engine::Background saveMenuBg;
};

#endif //UNDERTALE_SAVE_MENU_HPP
