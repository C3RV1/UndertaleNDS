//
// Created by cervi on 24/08/2022.
//

#include "Battle/Battle.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/DataBank.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Font.hpp"
#include "Engine/OAMManager.hpp"
#include "GameLoop.hpp"
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

  auto save = std::make_unique<SaveData>();
  save->loadData();

  runTitleScreen();
  if (!save->saveExists) {
    runWriteNameMenu(*save);
  } else {
    runMainMenu(*save);
  }

  for (int i = 0; i < 5; i++) {
    save->items[i] = 1 + (i % 3 == 0);
  }

  Engine::textMain.clear();
  Engine::textSub.clear();

  // DEBUG
  save->lastSavedRoom = 6;
  save->flags[0] = 5;

  runGameLoop(std::move(save));
  return 0;
}
