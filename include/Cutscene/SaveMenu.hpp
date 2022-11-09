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
    const int kNameX = 41, kNameY = 43 - 4;
    const int kLvNumX = 131, kLvNumY = 43 - 4;
    const int kRoomNameX = 41, kRoomNameY = 63 - 4;
    const int kHrtSaveX = 54, kHrtSaveY = 99;
    const int kHrtRetX = 148, kHrtRetY = 99;
    const int kHoldSaveFrames = 60;
    int _cHoldFrames = -1;
    int _selectedOption = 0;
    char* _roomName = nullptr;

    Audio::WAV _saveSnd;
    Engine::Texture _optionsHeartTex;
    Engine::Sprite _optionsHeartSpr;
    Engine::Font _fnt;
    Engine::Background _bg;
};

#endif //UNDERTALE_SAVE_MENU_HPP
