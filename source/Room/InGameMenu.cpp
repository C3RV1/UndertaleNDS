//
// Created by cervi on 01/09/2022.
//
#include "Room/InGameMenu.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"
#include <memory>

void InGameMenu::load() {
  _fnt = Engine::fontManager.loadFont("fnt_maintext.font");
  updateBg();

  Engine::spriteLoadTexture(_selectedMenuHeartSpr, "spr_heartsmall");
  Engine::spriteLoadTexture(_listHeartSpr, "spr_heartsmall");
}

void InGameMenu::unload() { hide(); }

void InGameMenu::hide() {
  if (!_shown)
    return;
  _shown = false;
  Engine::textSub.clear();
  Engine::clearSub();
  Engine::spriteSetShown(_selectedMenuHeartSpr, false);
  Engine::spriteSetShown(_listHeartSpr, false);
}

void InGameMenu::show() {
  if (_shown)
    return;

  _shown = true;
  Engine::textSub.clear();
  Engine::textSub.setColor(15);
  updateBg();
  updateSelectMenuHeart();
  drawName();
  drawLv();
  drawExp();
  updateHp();

  clipOption();
  if (_selectedMenu == MENU_ITEMS)
    drawItems();
  else
    drawCell();
}

void InGameMenu::updateBg() {
  _bgLoadedCell = globalSave.flags[FlagIds::OWNS_PHONE] == 1;

  if (_bgLoadedCell)
    _bg.loadPath("ingame_menu/bg");
  else
    _bg.loadPath("ingame_menu/bg_no_cell");

  if (_shown)
    _bg.loadBgTextSub();
}

void InGameMenu::updateSelectMenuHeart() {
  Engine::spriteSetShown(_selectedMenuHeartSpr, true);
  _selectedMenuHeartSpr->_wx =
      kSelectedMenuX + kSelectedMenuSeparation * _selectedMenu;
  _selectedMenuHeartSpr->_wy = kSelectedMenuY;
}

void InGameMenu::drawName() {
  int x = kNameX, y = kNameY;
  for (char *pName = globalSave.name; *pName != 0; pName++) {
    Engine::textSub.drawGlyph(_fnt, *pName, x, y);
  }
}

void InGameMenu::drawLv() {
  char buffer[16];
  sprintf(buffer, "%d", globalSave.lv);
  int x = kLvX, y = kLvY;
  for (char *pName = buffer; *pName != 0; pName++) {
    Engine::textSub.drawGlyph(_fnt, *pName, x, y);
  }
}

void InGameMenu::drawExp() {
  char buffer[16];
  sprintf(buffer, "%d", globalSave.exp);
  int x = kExpX, y = kExpY;
  for (char *pName = buffer; *pName != 0; pName++) {
    Engine::textSub.drawGlyph(_fnt, *pName, x, y);
  }
}

void InGameMenu::drawItems() {
  if (globalSave.items[0] == 0) {
    Engine::spriteSetShown(_listHeartSpr, false);
  } else {
    drawItemPage();
    drawItemDesc();
    Engine::spriteSetShown(_listHeartSpr, true);
    setItemHeartPos();
  }
}

void InGameMenu::drawItemExplain() {
  Engine::textSub.drawHollowRect(17 - 2, 102 - 2, 222 + 4, 57 + 4, 2, 15);
  Engine::textSub.drawRect(17, 102, 222, 57, 0);
}

void InGameMenu::clipOption() {
  if (_selectedMenu == MENU_ITEMS) {
    for (_optionCount = 0; globalSave.items[_optionCount] != 0; _optionCount++)
      ;
    _pageCount = ((_optionCount - 1) / 2) + 1;
    if (_itemPage > _pageCount - 1)
      _itemPage = _pageCount - 1;
    if (_optionSelected > _optionCount - _itemPage * 2 - 1)
      _optionSelected = _optionCount - _itemPage * 2 - 1;
  } else {
    for (_optionCount = 0; globalSave.cell[_optionCount] != 0; _optionCount++)
      ;
    if (_optionSelected > _optionCount - 1)
      _optionSelected = _optionCount - 1;
  }
}

void InGameMenu::drawItemPage() {
  char buffer[100];
  int x = 5;
  if (_itemPage > 0)
    Engine::textSub.drawGlyph(_fnt, '<', x, kPageChangeY);
  else
    Engine::textSub.drawGlyph(_fnt, ' ', x, kPageChangeY);

  x = 256 - 15;
  if (_itemPage < (_optionCount - 1) / 2)
    Engine::textSub.drawGlyph(_fnt, '>', x, kPageChangeY);
  else
    Engine::textSub.drawGlyph(_fnt, ' ', x, kPageChangeY);

  int y = kItemsY;
  for (int i = 0; i < 2; i++) {
    int itemIdx = (_itemPage * 2) + i;
    if (itemIdx >= _optionCount)
      break;
    int item = globalSave.items[itemIdx];

    sprintf(buffer, "nitro:/txt/items/name%d.txt", item);
    FILE *f = fopen(buffer, "rb");
    int len = str_len_file(f, '\n');
    fread(buffer, len + 1, 1, f);
    fclose(f);
    x = kItemsX;
    for (char *pName = buffer; *pName != '\n'; pName++) {
      Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }
    y += kItemSpacingY;
  }
}

void InGameMenu::setItemHeartPos() {
  _listHeartSpr->_wx = (kItemsX - 12) << 8;
  _listHeartSpr->_wy = (kItemsY + _optionSelected * kItemSpacingY + 4) << 8;
}

void InGameMenu::drawItemDesc() {
  drawItemExplain();
  char buffer[100];
  int itemIdx = _itemPage * 2 + _optionSelected;
  int item = globalSave.items[itemIdx];
  sprintf(buffer, "nitro:/txt/items/desc%d.txt", item);
  FILE *f = fopen(buffer, "rb");
  int len = str_len_file(f, '\0');
  fread(buffer, len + 1, 1, f);
  buffer[len] = '\0';
  fclose(f);
  int x = 23, y = 104;
  for (char *pName = buffer; *pName != '\0'; pName++) {
    if (*pName == '\n') {
      y += 17;
      x = 23;
      continue;
    }
    Engine::textSub.drawGlyph(_fnt, *pName, x, y);
  }
}

void InGameMenu::drawCell() {
  clipOption();
  drawCellPage();
  Engine::spriteSetShown(_listHeartSpr, true);
  setItemHeartPos();
}

void InGameMenu::drawCellPage() {
  char buffer[100];
  int x, y = kItemsY;

  for (int i = 0; i < _optionCount; i++) {
    int cellOption = globalSave.cell[i];

    sprintf(buffer, "nitro:/txt/cell/name%d.txt", cellOption);
    FILE *f = fopen(buffer, "rb");
    int len = str_len_file(f, '\n');
    fread(buffer, len + 1, 1, f);
    fclose(f);

    x = kItemsX;
    for (char *pName = buffer; *pName != '\n'; pName++) {
      Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }
    y += kItemSpacingY;
  }
}

void InGameMenu::pageClear() {
  Engine::textSub.clearRect(0, kItemsY, 256, kItemsH);
}

void InGameMenu::updateHp() {
  if (!_shown)
    return;

  char buffer[16];

  sprintf(buffer, "%d/%d", globalSave.hp, globalSave.maxHp);
  int x = kHpX, y = kHpY;
  for (char *p = buffer; *p != 0; p++)
    Engine::textSub.drawGlyph(_fnt, *p, x, y);
}

void InGameMenu::update() {
  if (!_shown)
    return;
  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);
    if (_selectedMenu == MENU_ITEMS)
      processTouchItems(touch);
    else
      processTouchCell(touch);
  }
}

void InGameMenu::processTouchItems(touchPosition &touch) {
  if (touch.px > 140 && touch.px < 140 + 58 && touch.py > 35 &&
      touch.py < 35 + 19) {
    if (_selectedMenu != MENU_CELL &&
        globalSave.flags[FlagIds::OWNS_PHONE] == 1) {
      _selectedMenu = MENU_CELL;
      updateSelectMenuHeart();
      _optionSelected = 0;
      pageClear();
      drawCell();
    }
  } else if (touch.py > 35 + 19 && touch.py < kItemsY + kItemSpacingY * 2) {
    if (touch.px < kItemsX + kButtonWidth && touch.px > kItemsX) {
      int itemY = (touch.py - kItemsY) / kItemSpacingY;
      int itemIdx = itemY;
      if (itemIdx != _optionSelected &&
          _itemPage * 2 + _optionSelected < _optionCount) {
        _optionSelected = itemIdx;
        clipOption();
        setItemHeartPos();
        drawItemDesc();
      }
    } else if (touch.py > kPageChangeY - 5 && touch.py < kPageChangeY + 20) {
      if (touch.px < 15) {
        if (_itemPage > 0) {
          _itemPage--;
          pageClear();
          clipOption();
          drawItemPage();
          setItemHeartPos();
          drawItemDesc();
        }
      } else if (touch.px > 256 - 25) {
        if (_itemPage < _pageCount - 1) {
          _itemPage++;
          pageClear();
          clipOption();
          drawItemPage();
          setItemHeartPos();
          drawItemDesc();
        }
      }
    }
  }
}

void InGameMenu::processTouchCell(touchPosition &touch) {
  if (touch.px > 53 && touch.px < 53 + 58 && touch.py > 35 &&
      touch.py < 35 + 19) {
    if (_selectedMenu != MENU_ITEMS) {
      _selectedMenu = MENU_ITEMS;
      updateSelectMenuHeart();
      _optionSelected = 0;
      _itemPage = 0;
      pageClear();
      drawItems();
    }
  } else if (touch.px > kItemsX && touch.py > kItemsY &&
             touch.py < kItemsY + kItemSpacingY * (_optionCount + 1)) {
    int touchedOption = (touch.py - kItemsY) / kItemSpacingY;
    if (touchedOption != _optionSelected) {
      _optionSelected = touchedOption;
      for (_optionCount = 0; globalSave.cell[_optionCount] != 0; _optionCount++)
        ;
      if (_optionSelected > _optionCount - 1)
        _optionSelected = _optionCount - 1;
      setItemHeartPos();
    } else {
      // Room 1000 for phone cutscenes
      if (globalCutscene == nullptr)
        globalCutscene =
            std::make_unique<Cutscene>(globalSave.cell[touchedOption], 1000);
    }
  }
}

InGameMenu globalInGameMenu;
