//
// Created by cervi on 24/08/2022.
//

#include <cstdio>
#include "Engine/Engine.hpp"
#include "Engine/Font.hpp"
#include "Engine/FreeZoneManager.hpp"
#include "TitleScreen.hpp"
#include "WriteName.hpp"
#include "MainMenu.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Save.hpp"

int main() {
    /* Configure the VRAM and background control registers. */
    if (Engine::init() != 0)
        return 0;

    char buffer[1024];
    int timeTaken[100]{0};
    FILE *f = fopen("nitro:/z_audio/mus_ruins.wav", "rb");
    for (int & i : timeTaken) {
        timerStart(2, ClockDivider::ClockDivider_64, 0, nullptr);
        for (int j = 0; j < 10; j++) {
            fread(buffer, 1024, 1, f);
        }
        timerPause(2);
        i = timerElapsed(2);
    }

    int sum = 0;
    for (int & i : timeTaken) {
        sum += i / 100;
    }
    std::string res = "Taken, on average, " + std::to_string(sum) + " ticks.";
    nocashMessage(res.c_str());
    while(true) {
        Engine::tick();
    }

    globalSave.loadData();

    runTitleScreen();
    if (!globalSave.saveExists) {
        runWriteNameMenu();
    } else {
        runMainMenu();
    }

    /* for (int i = 0; i < 5; i++) {
        globalSave.items[i] = 1 + (i % 3 == 0);
    }*/

    Engine::textMain.clear();
    Engine::textSub.clear();

    u16 roomSpawn = globalSave.lastSavedRoom;

    // DEBUG
    roomSpawn = 5;
    globalSave.flags[0] = 4;

    globalPlayer = new Player();
    globalPlayer->_playerSpr.setShown(true);
    globalInGameMenu.load();
    globalInGameMenu.show(false);
    globalRoom = std::make_unique<Room>(roomSpawn);
    globalCamera.updatePosition(true);
    globalPlayer->_playerSpr._wx = globalRoom->_spawnX << 8;
    globalPlayer->_playerSpr._wy = globalRoom->_spawnY << 8;

    for (;;) {
        Engine::tick();
        globalPlayer->update();
        globalRoom->update();
        if (globalCutscene != nullptr) {
            globalCutscene->update();
            if (globalCutscene->runCommands(ROOM)) {
                globalCutscene = nullptr;
                globalInGameMenu.show(false);
                globalPlayer->setPlayerControl(true);
                globalCamera._manual = false;
            }
        }
        globalCamera.updatePosition(false);
        globalInGameMenu.update();
        globalPlayer->draw();
        globalRoom->draw();
    }

    return 0;
}