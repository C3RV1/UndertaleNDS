//
// Created by cervi on 28/09/2022.
//
#include "Cutscene/SaveMenu.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Formats/utils.hpp"

SaveMenu::SaveMenu() : optionsHeart(Engine::AllocatedOAM) {
    char buffer[100];
    FILE *f = fopen("nitro:/fnt/fnt_maintext.font.cfnt", "rb");
    if (f) {
        int fontLoad = font.loadCFNT(f);
        if (fontLoad != 0) {
            sprintf(buffer, "Error loading font: %d", fontLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening font file");
    }
    fclose(f);

    f = fopen("nitro:/bg/save_menu_bg.cbgf", "rb");
    if (f) {
        int bgLoad = saveMenuBg.loadCBGF(f);
        if (bgLoad != 0) {
            sprintf(buffer, "Error loading bg: %d", bgLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening bg file");
    }
    fclose(f);
    Engine::loadBgTextSub(saveMenuBg);

    f = fopen("nitro:/spr/spr_heartsmall.cspr", "rb");
    if (f) {
        int sprLoad = optionsHeartTex.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading spr: %d", sprLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening heart spr file");
    }
    fclose(f);
    optionsHeart.loadTexture(optionsHeartTex);
    optionsHeart.setShown(true);
    optionsHeart.wx = hrtSaveX << 8;
    optionsHeart.wy = hrtSaveY << 8;

    saveSnd.loadWAV("snd_save.wav");
    saveSnd.setLoops(0);

    SaveData lastSave;
    lastSave.loadData();
    drawInfo(lastSave, 15);
}

void SaveMenu::drawInfo(SaveData& saveData, u8 color) {
    Engine::textSub.clear();

    Engine::textSub.setCurrentColor(15);

    if (!saveData.saveExists) {
        int x = nameX;
        Engine::textSub.drawGlyph(font, '-', x, nameY);
        x = lvNumX;
        Engine::textSub.drawGlyph(font, '0', x, lvNumY);
        x = roomNameX;
        Engine::textSub.drawGlyph(font, '-', x, roomNameY);
        return;
    }

    char buffer[100];
    sprintf(buffer, "nitro:/data/rooms/names/%d.txt", saveData.lastSavedRoom);
    FILE *f = fopen(buffer, "rb");
    if (f) {
        delete[] roomName;

        int len = strlen_file(f, '\n');
        roomName = new char[len + 1];

        fread(roomName, len, 1, f);
        roomName[len] = 0;
    } else {
        nocashMessage("Error opening room name file");
    }
    fclose(f);

    sprintf(buffer, "%d", saveData.lv);
    int x = lvNumX;
    for (char *p = buffer; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, lvNumY);
    }

    Engine::textSub.setCurrentColor(color);

    x = roomNameX;
    for (char *p = roomName; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, roomNameY);
    }

    x = nameX;
    for (char *p = saveData.name; *p != 0; p++) {
        Engine::textSub.drawGlyph(font, *p, x, nameY);
    }
}

bool SaveMenu::update() {
    if (currentHoldFrames > 0) {
        currentHoldFrames--;
        if (currentHoldFrames == 0) {
            return true;
        }
        return false;
    }
    if (selectedOption == 0 && keysDown() & KEY_RIGHT)
        selectedOption = 1;
    else if (keysDown() & KEY_LEFT)
        selectedOption = 0;

    if (keysDown() & KEY_A) {
        if (selectedOption == 1)
            return true;
        globalSave.saveData(globalCutscene->roomId);
        saveSnd.play();
        drawInfo(globalSave, 12);
        currentHoldFrames = holdSaveFrames;
    }

    if (selectedOption == 0) {
        optionsHeart.wx = hrtSaveX << 8;
        optionsHeart.wy = hrtSaveY << 8;
    } else {
        optionsHeart.wx = hrtRetX << 8;
        optionsHeart.wy = hrtRetY << 8;
    }
    return false;
}

void SaveMenu::free_() {
    delete[] roomName;
    roomName = nullptr;
    saveSnd.stop();
    saveSnd.free_();
    optionsHeart.setShown(false);
    optionsHeartTex.free_();
    font.free_();
    saveMenuBg.free_();
    Engine::clearSub();
}
