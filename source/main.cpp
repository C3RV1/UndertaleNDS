//
// Created by cervi on 24/08/2022.
//

#include "Cutscene/Cutscene.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Font.hpp"
#include "Engine/OAMManager.hpp"
#include "MainMenu.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include "Save.hpp"
#include "TitleScreen.hpp"
#include "WriteName.hpp"
#include <cstdio>

int main() {
  defaultExceptionHandler();

  /* Configure the VRAM and background control registers. */
  if (Engine::init() != 0)
    return 0;

  globalSave.loadData();

  runTitleScreen();
  if (!globalSave.saveExists) {
    runWriteNameMenu();
  } else {
    runMainMenu();
  }

  for (int i = 0; i < 5; i++) {
    globalSave.items[i] = 1 + (i % 3 == 0);
  }

  Engine::textMain.clear();
  Engine::textSub.clear();

  u16 roomSpawn = globalSave.lastSavedRoom;

  // DEBUG
  roomSpawn = 8;
  globalSave.flags[0] = 9;

  globalPlayer = new Player();
  Engine::spriteSetShown(globalPlayer->_playerSpr, true);
  globalInGameMenu.load();
  globalInGameMenu.show();
  globalRoom = std::make_unique<Room>(roomSpawn);
  globalCamera.updatePosition(true);
  globalPlayer->_playerSpr->_wx = globalRoom->_spawnX << 8;
  globalPlayer->_playerSpr->_wy = globalRoom->_spawnY << 8;

  for (;;) {
    Engine::tick();
    globalPlayer->update();
    globalRoom->update();
    if (globalCutscene != nullptr) {
      globalCutscene->update();
      if (globalCutscene->runCommands(ROOM)) {
        globalCutscene = nullptr;
        globalInGameMenu.show();
        globalPlayer->set_player_control(true);
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
