//
// Created by cervi on 24/08/2022.
//

#include "Battle/Battle.hpp"
#include "BootScreens/SplashScreen.hpp"
#include "Engine/DataBank.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Font.hpp"
#include "GameLoop.hpp"
#include "MainMenu.hpp"
#include "Room/InGameMenu.hpp"
#include "Save.hpp"
#include "BootScreens/TitleScreen.hpp"
#include "BootScreens/WriteName.hpp"
#include "BootScreens/SaveErrorScreen.hpp"
#include <cstdio>
#include <memory>


int main() {
  /* Configure VRAM, background control registers, exceptions, filesystem, 3d,
   * audio. */
  if (Engine::init() != 0)
    return 0;

  textBank.load("nitro:/txts.cbnk");

  runSplashScreen();
  runTitleScreen();
  
  auto save = std::make_unique<SaveData>();
  if (!save->loadData())
    runSaveErrorScreen(*save);
  
  if (!save->saveExists) {
    runWriteNameMenu(*save);
  } else {
    runMainMenu(*save);
  }

  Engine::textMain.clear();
  Engine::textSub.clear();

  // DEBUG
  // save->lastSavedRoom = 6;
  // save->flags[0] = 5;
  
  for (int i = 0; i < 5; i++) {
    save->items[i] = 1 + (i % 3 == 0);
  }

  runGameLoop(std::move(save));
  return 0;
}
