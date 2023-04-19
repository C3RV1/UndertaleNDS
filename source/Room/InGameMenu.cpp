//
// Created by cervi on 01/09/2022.
//
#include "Cutscene/Cutscene.hpp"
#include "Room/InGameMenu.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"

void InGameMenu::load() {
    _fnt.loadPath("fnt_maintext.font");
    _bgLoadedCell = globalSave.flags[2] == 1;
    if (_bgLoadedCell)
        _bg.loadPath("ingame_menu/bg");
    else
        _bg.loadPath("ingame_menu/bg_no_cell");

    _littleHeartTex.loadPath("spr_heartsmall");
    _itemExplainTex.loadPath("ingame_menu/item_explain");

    _selectedMenuHeartSpr.loadTexture(_littleHeartTex);
    _listHeartSpr.loadTexture(_littleHeartTex);
    _itemExplainBoxSpr.loadTexture(_itemExplainTex);
    _itemExplainBoxSpr._wx = 17 << 8;
    _itemExplainBoxSpr._wy = 102 << 8;
}

void InGameMenu::unload() {
    hide();
}

void InGameMenu::hide() {
    if (!_shown)
        return;
    _shown = false;
    Engine::textSub.clear();
    Engine::clearSub();
    _selectedMenuHeartSpr.setShown(false);
    _listHeartSpr.setShown(false);
    _itemExplainBoxSpr.setShown(false);
}

void InGameMenu::show(bool update) {
    // TODO: Make Font so we don't have to render all text again
    //       when we change the selected menu (consumes a lot)
    //       Or maybe optimize? Or maybe don't do anything, it
    //       only runs for a single frame so who cares?
    if (_shown && !update)
        return;

    if (globalSave.flags[2] == 1 && !_bgLoadedCell) {
        _bg.loadPath("ingame_menu/bg");
        _bgLoadedCell = true;
    }

    _shown = true;
    if (!update)
        _bg.loadBgTextSub();
    Engine::textSub.clear();
    Engine::textSub.setColor(15);
    _selectedMenuHeartSpr.setShown(true);
    _selectedMenuHeartSpr._wx = kSelectedMenuX + kSelectedMenuSeparation * _selectedMenu;
    _selectedMenuHeartSpr._wy = kSelectedMenuY;

    char buffer[200];
    int x = kNameX, y = kNameY;
    for (char* pName = globalSave.name; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }

    sprintf(buffer, "%d/%d", globalSave.hp, globalSave.maxHp);
    x = kHpX, y = kHpY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", globalSave.lv);
    x = kLvX, y = kLvY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", globalSave.exp);
    x = kExpX, y = kExpY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(_fnt, *pName, x, y);
    }

    if (_selectedMenu == MENU_ITEMS) {
        if (globalSave.items[0] == 0) {
            _listHeartSpr.setShown(false);
            _itemExplainBoxSpr.setShown(false);
        } else {
            y = kItemsY;
            for (_optionCount = 0; globalSave.items[_optionCount] != 0; _optionCount++);
            _pageCount = ((_optionCount - 1) / 2) + 1;
            if (_itemPage > _pageCount - 1)
                _itemPage = _pageCount - 1;
            if (_optionSelected > _optionCount - _itemPage * 2 - 1)
                _optionSelected = _optionCount - _itemPage * 2 - 1;
            if (_itemPage > 0) {
                x = 5;
                Engine::textSub.drawGlyph(_fnt, '<', x, kPageChangeY);
            }
            if (_itemPage < (_optionCount - 1) / 2) {
                x = 256 - 15;
                Engine::textSub.drawGlyph(_fnt, '>', x, kPageChangeY);
            }
            for (int i = 0; i < 2; i++) {
                int itemIdx = (_itemPage * 2) + i;
                if (itemIdx >= _optionCount)
                    break;
                int item = globalSave.items[itemIdx];

                sprintf(buffer, "nitro:/data/items/name%d.txt", item);
                FILE* f = fopen(buffer, "rb");
                int len = str_len_file(f, '\n');
                fread(buffer, len + 1, 1, f);
                fclose(f);
                x = kItemsX;
                if (i == _optionSelected) {
                    _listHeartSpr._wx = (x - 12) << 8;
                    _listHeartSpr._wy = (y + 4) << 8;
                }
                for (char* pName = buffer; *pName != '\n'; pName++) {
                    Engine::textSub.drawGlyph(_fnt, *pName, x, y);
                }
                y += kItemSpacingY;
            }

            _listHeartSpr.setShown(true);
            _itemExplainBoxSpr.setShown(true);

            // TODO: Make descriptions have multiple pages (ex. temmie armor)
            int itemIdx = _itemPage * 2 + _optionSelected;
            int item = globalSave.items[itemIdx];
            sprintf(buffer, "nitro:/data/items/desc%d.txt", item);
            FILE* f = fopen(buffer, "rb");
            int len = str_len_file(f, '\0');
            fread(buffer, len + 1, 1, f);
            buffer[len] = '\0';
            fclose(f);
            x = 23, y = 106;
            for (char* pName = buffer; *pName != '\0'; pName++) {
                if (*pName == '\n') {
                    y += 15;
                    x = 23;
                    continue;
                }
                Engine::textSub.drawGlyph(_fnt, *pName, x, y);
            }
        }
    } else {
        // CELL menu
        _listHeartSpr.setShown(true);
        _itemExplainBoxSpr.setShown(false);
        for (_optionCount = 0; globalSave.cell[_optionCount] != 0; _optionCount++);
        if (_optionSelected > _optionCount - 1)
            _optionSelected = _optionCount - 1;
        y = kItemsY;
        for (int i = 0; i < _optionCount; i++) {
            int cellOption = globalSave.cell[i];

            sprintf(buffer, "nitro:/data/cell/name%d.txt", cellOption);
            FILE* f = fopen(buffer, "rb");
            int len = str_len_file(f, '\n');
            fread(buffer, len + 1, 1, f);
            fclose(f);
            x = kItemsX;
            if (i == _optionSelected) {
                _listHeartSpr._wx = (x - 12) << 8;
                _listHeartSpr._wy = (y + 4) << 8;
            }
            for (char* pName = buffer; *pName != '\n'; pName++) {
                Engine::textSub.drawGlyph(_fnt, *pName, x, y);
            }
            y += kItemSpacingY;
        }
    }
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
    if (touch.px > 140 && touch.px < 140 + 58 && touch.py > 35 && touch.py < 35 + 19) {
        if (_selectedMenu != MENU_CELL && globalSave.flags[2] == 1) {
            _selectedMenu = MENU_CELL;
            show(true);
        }
    }
    else if (touch.py > 35 + 19 && touch.py < kItemsY + kItemSpacingY * 2) {
        if (touch.px < kItemsX + kButtonWidth &&
            touch.px > kItemsX) {
            int itemY = (touch.py - kItemsY) / kItemSpacingY;
            int itemIdx = itemY;
            if (itemIdx != _optionSelected && _itemPage * 2 + _optionSelected < _optionCount) {
                _optionSelected = itemIdx;
                show(true);
            }
        } else if (touch.py > kPageChangeY - 5 && touch.py < kPageChangeY + 20) {
            if (touch.px < 15) {
                if (_itemPage > 0) {
                    _itemPage--;
                    show(true);
                }
            }
            else if (touch.px > 256 - 25) {
                if (_itemPage < _pageCount - 1) {
                    _itemPage++;
                    show(true);
                }
            }
        }
    }
}

void InGameMenu::processTouchCell(touchPosition &touch) {
    if (touch.px > 53 && touch.px < 53 + 58 && touch.py > 35 && touch.py < 35 + 19) {
        if (_selectedMenu != MENU_ITEMS) {
            _selectedMenu = MENU_ITEMS;
            _optionSelected = 0;
            _itemPage = 0;
            show(true);
        }
    } else if (touch.px > kItemsX && touch.py > kItemsY && touch.py < kItemsY + kItemSpacingY * (_optionCount + 1)) {
        int touchedOption = (touch.py - kItemsY) / kItemSpacingY;
        if (touchedOption != _optionSelected) {
            _optionSelected = touchedOption;
            show(true);
        } else {
            // Room 1000 for phone cutscenes
            if (globalCutscene == nullptr)
                globalCutscene = new Cutscene(globalSave.cell[touchedOption],
                                              1000);
        }
    }
}

InGameMenu globalInGameMenu;
