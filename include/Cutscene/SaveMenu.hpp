//
// Created by cervi on 28/09/2022.
//

#ifndef UNDERTALE_SAVE_MENU_HPP
#define UNDERTALE_SAVE_MENU_HPP

#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/WAV.hpp"
#include "Save.hpp"
#include <memory>
#include <nds.h>

class SaveMenu {
public:
  SaveMenu();
  ~SaveMenu() { free_(); }
  void drawInfo(SaveData &saveData, u8 color);
  bool update();

private:
  void free_();
  constexpr static int kNameX = 41, kNameY = 43 - 4;
  constexpr static int kLvNumX = 131, kLvNumY = 43 - 4;
  constexpr static int kRoomNameX = 41, kRoomNameY = 63 - 4;
  constexpr static int kHrtSaveX = 54, kHrtSaveY = 99;
  constexpr static int kHrtRetX = 148, kHrtRetY = 99;
  constexpr static int kHoldSaveFrames = 60;
  int _cHoldFrames = -1;
  int _selectedOption = 0;
  std::string _roomName;

  std::shared_ptr<Audio2::WAV> _saveSnd = nullptr;
  Engine::Texture _optionsHeartTex;
  Engine::Sprite _optionsHeartSpr;
  Engine::Font _fnt;
  Engine::Background _bg;
};

#endif // UNDERTALE_SAVE_MENU_HPP
