//
// Created by cervi on 24/08/2022.
//

#include "Battle/Battle.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/DataBank.hpp"
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
#include <memory>

void runBattle();
void runRoom();

int main() {
  defaultExceptionHandler();

  /* Configure the VRAM and background control registers. */
  if (Engine::init() != 0)
    return 0;

  textBank.load("nitro:/txts.cbnk");

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
  roomSpawn = 6;
  globalSave.flags[0] = 5;

  globalPlayer = std::make_unique<Player>();
  Engine::spriteSetShown(globalPlayer->_spr, true);
  globalInGameMenu.load();
  globalInGameMenu.show();
  globalRoom = std::make_unique<Room>(roomSpawn);
  globalCamera.updatePosition(true);
  globalPlayer->_spr->_wx = globalRoom->_spawnX << 8;
  globalPlayer->_spr->_wy = globalRoom->_spawnY << 8;

  for (;;) {
    if (globalBattle == nullptr)
      runRoom();
    else
      runBattle();
  }

  return 0;
}

void runRoom() {
  while (globalBattle == nullptr) {
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
}

void runBattle() {
  int timer = kRoomChangeFadeFrames;
  while (timer >= 0) {
    Engine::tick();
    setBrightness(3, (-16 * (kRoomChangeFadeFrames - timer)) /
                         kRoomChangeFadeFrames);
    timer--;
  }

  globalRoom->push();
  Engine::textMain.clear();
  Engine::textSub.clear();
  globalInGameMenu.unload();

  lcdMainOnBottom();

  globalBattle->enter();

  if (globalCutscene != nullptr) {
    globalCutscene->runCommands(LOAD_BATTLE);
  }

  timer = kRoomChangeFadeFrames;
  while (timer >= 0) {
    Engine::tick();
    globalBattle->update();
    setBrightness(3, (-16 * timer) / kRoomChangeFadeFrames);
    timer--;
  }

  while (globalBattle->_running) {
    Engine::tick();
    if (globalCutscene != nullptr) {
      if (globalBattle->_stopPostDialogue &&
          globalCutscene->_cDialogue == nullptr) {
        globalBattle->_running = false;
      }
      globalCutscene->update();
      if (globalCutscene->runCommands(BATTLE)) {
        globalCutscene = nullptr;
        globalInGameMenu.show();
        globalPlayer->set_player_control(true);
        globalCamera._manual = false;
      }
    }
    globalBattle->update();
  }

  timer = kRoomChangeFadeFrames;
  while (timer >= 0) {
    Engine::tick();
    globalBattle->update();
    setBrightness(3, (-16 * (kRoomChangeFadeFrames - timer)) /
                         kRoomChangeFadeFrames);
    timer--;
  }

  globalBattle = nullptr;
  Engine::textMain.clear();
  Engine::textSub.clear();
  Engine::clearSub();

  lcdMainOnTop();

  globalRoom->pop();
  globalInGameMenu.load();

  if (globalCutscene != nullptr) {
    globalCutscene->runCommands(LOAD_ROOM);
  }
  globalCamera.updatePosition(true);
  globalPlayer->draw();
  globalRoom->draw();

  timer = kRoomChangeFadeFrames;
  while (timer >= 0) {
    Engine::tick();
    setBrightness(3, (-16 * timer) / kRoomChangeFadeFrames);
    timer--;
  }
}
