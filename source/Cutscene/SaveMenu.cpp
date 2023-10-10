//
// Created by cervi on 28/09/2022.
//
#include "Cutscene/SaveMenu.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Formats/utils.hpp"

SaveMenu::SaveMenu() : _optionsHeartSpr(Engine::AllocatedOAM) {
    _fnt.loadPath("fnt_maintext.font");

    _bg.loadPath("save_menu_bg");
    _bg.loadBgTextSub();

    _optionsHeartTex.loadPath("spr_heartsmall");
    _optionsHeartSpr.loadTexture(_optionsHeartTex);
    _optionsHeartSpr.setShown(true);
    _optionsHeartSpr._wx = kHrtSaveX << 8;
    _optionsHeartSpr._wy = kHrtSaveY << 8;

    _saveSnd.load("snd_save.wav");
    _saveSnd.setLoops(0);

    SaveData lastSave;
    lastSave.loadData();
    drawInfo(lastSave, 15);
}

void SaveMenu::drawInfo(SaveData& saveData, u8 color) {
    Engine::textSub.clear();

    Engine::textSub.setColor(15);

    if (!saveData.saveExists) {
        int x = kNameX;
        Engine::textSub.drawGlyph(_fnt, '-', x, kNameY);
        x = kLvNumX;
        Engine::textSub.drawGlyph(_fnt, '0', x, kLvNumY);
        x = kRoomNameX;
        Engine::textSub.drawGlyph(_fnt, '-', x, kRoomNameY);
        return;
    }

    std::string buffer = "nitro:/data/room_names/" + std::to_string(saveData.lastSavedRoom) +
            ".txt";
    FILE *f = fopen(buffer.c_str(), "rb");
    if (f) {
        int len = str_len_file(f, '\n');
        _roomName.resize(len);

        fread(&_roomName[0], len, 1, f);
    } else {
        nocashMessage("Error opening room name file");
    }
    fclose(f);

    buffer = std::to_string(saveData.lv);
    int x = kLvNumX;
    for (auto const & c : buffer) {
        Engine::textSub.drawGlyph(_fnt, c, x, kLvNumY);
    }

    Engine::textSub.setColor(color);

    x = kRoomNameX;
    for (auto const & c : _roomName) {
        Engine::textSub.drawGlyph(_fnt, c, x, kRoomNameY);
    }

    x = kNameX;
    for (char *p = saveData.name; *p != 0; p++) {
        Engine::textSub.drawGlyph(_fnt, *p, x, kNameY);
    }
}

bool SaveMenu::update() {
    if (_cHoldFrames > 0) {
        _cHoldFrames--;
        if (_cHoldFrames == 0) {
            return true;
        }
        return false;
    }
    if (_selectedOption == 0 && keysDown() & KEY_RIGHT)
        _selectedOption = 1;
    else if (keysDown() & KEY_LEFT)
        _selectedOption = 0;

    if (keysDown() & KEY_A) {
        if (_selectedOption == 1)
            return true;
        globalSave.saveData(globalCutscene->_roomId);
        _saveSnd.play();
        drawInfo(globalSave, 12);
        _cHoldFrames = kHoldSaveFrames;
    }

    if (_selectedOption == 0) {
        _optionsHeartSpr._wx = kHrtSaveX << 8;
        _optionsHeartSpr._wy = kHrtSaveY << 8;
    } else {
        _optionsHeartSpr._wx = kHrtRetX << 8;
        _optionsHeartSpr._wy = kHrtRetY << 8;
    }
    return false;
}

void SaveMenu::free_() {
    Engine::clearSub();
}
