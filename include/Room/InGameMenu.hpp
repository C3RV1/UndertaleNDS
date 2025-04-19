//
// Created by cervi on 01/09/2022.
//

#ifndef UNDERTALE_INGAME_MENU_HPP
#define UNDERTALE_INGAME_MENU_HPP

#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include <memory>

enum SelectedMenu { MENU_ITEMS, MENU_CELL };

class InGameMenu {
public:
  InGameMenu() {
    _selectedMenuHeartSpr =
        std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
    _listHeartSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  }
  void load();
  void unload();
  void updateHp();
  void show();
  void updateBg();
  void updateSelectMenuHeart();
  void drawName();
  void drawLv();
  void drawExp();
  void drawItems();
  void drawCell();
  void drawCellPage();
  void pageClear();
  void clipOption();
  void drawItemPage();
  void drawItemDesc();
  void drawItemExplain();
  void setItemHeartPos();
  void processTouchItems(touchPosition &touch);
  void processTouchCell(touchPosition &touch);
  void hide();
  void update();

private:
  constexpr static int kNameX = 18, kNameY = 10;
  constexpr static int kHpX = 120, kHpY = 10;
  constexpr static int kLvX = 226, kLvY = 10;
  constexpr static int kExpX = 226, kExpY = 22;
  constexpr static int kSelectedMenuX = 59 << 8, kSelectedMenuY = 40 << 8;
  constexpr static int kSelectedMenuSeparation = (146 - 59) << 8;
  constexpr static int kItemsX = 58, kItemsY = 58, kItemSpacingY = 17;
  constexpr static int kItemsW = 256, kItemsH = 192 - kItemsY;
  const int kPageChangeY = kItemsY + kItemSpacingY - kItemSpacingY / 2;
  const int kButtonWidth = 90;

  bool _shown = false;
  std::shared_ptr<Engine::Font> _fnt;
  Engine::Background _bg;
  std::shared_ptr<Engine::Sprite> _selectedMenuHeartSpr, _listHeartSpr;
  SelectedMenu _selectedMenu = MENU_ITEMS;
  u8 _optionCount = 0;
  u8 _pageCount = 0;
  u8 _optionSelected = 0;
  u8 _itemPage = 0;

  bool _bgLoadedCell = false;
};

extern InGameMenu globalInGameMenu;

#endif // UNDERTALE_INGAME_MENU_HPP
