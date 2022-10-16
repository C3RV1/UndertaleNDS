//
// Created by cervi on 01/09/2022.
//
#include "Cutscene/Cutscene.hpp"
#include "Room/InGameMenu.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"

void InGameMenu::load() {
    fnt.loadPath("fnt_maintext.font");
    bgLoadedCell = globalSave.flags[2] == 1;
    if (bgLoadedCell)
        bg.loadPath("ingame_menu/bg");
    else
        bg.loadPath("ingame_menu/bg_no_cell");

    littleHeart.loadPath("spr_heartsmall");
    itemExplain.loadPath("ingame_menu/item_explain");

    selectedMenuHeart.loadTexture(littleHeart);
    listHeart.loadTexture(littleHeart);
    itemExplainBox.loadTexture(itemExplain);
    itemExplainBox.wx = 17 << 8;
    itemExplainBox.wy = 102 << 8;
}

void InGameMenu::unload() {
    hide();
    fnt.free_();
    littleHeart.free_();
    itemExplain.free_();
}

void InGameMenu::hide() {
    if (!shown)
        return;
    shown = false;
    Engine::textSub.clear();
    Engine::clearSub();
    selectedMenuHeart.setShown(false);
    listHeart.setShown(false);
    itemExplainBox.setShown(false);
}

void InGameMenu::show(bool update) {
    // TODO: Make Font so we don't have to render all text again
    //       when we change the selected menu (consumes a lot)
    //       Or maybe optimize? Or maybe don't do anything, it
    //       only runs for a single frame so who cares?
    if (shown && !update)
        return;

    if (globalSave.flags[2] == 1 && !bgLoadedCell) {
        bg.loadPath("ingame_menu/bg");
        bgLoadedCell = true;
    }

    shown = true;
    if (!update)
        Engine::loadBgTextSub(bg);
    Engine::textSub.clear();
    Engine::textSub.setCurrentColor(15);
    selectedMenuHeart.setShown(true);
    selectedMenuHeart.wx = selectedMenuX + selectedMenuSeparation * selectedMenu;
    selectedMenuHeart.wy = selectedMenuY;

    char buffer[200];
    int x = nameX, y = nameY;
    for (char* pName = globalSave.name; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d/%d", globalSave.hp, globalSave.maxHp);
    x = hpX, y = hpY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", globalSave.lv);
    x = lvX, y = lvY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", globalSave.exp);
    x = expX, y = expY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    if (selectedMenu == MENU_ITEMS) {
        if (globalSave.items[0] == 0) {
            listHeart.setShown(false);
            itemExplainBox.setShown(false);
        } else {
            y = itemsY;
            for (optionCount = 0; globalSave.items[optionCount] != 0; optionCount++);
            pageCount = ((optionCount - 1) / 2) + 1;
            if (itemPage > pageCount - 1)
                itemPage = pageCount - 1;
            if (optionSelected > optionCount - itemPage * 2 - 1)
                optionSelected = optionCount - itemPage * 2 - 1;
            if (itemPage > 0) {
                x = 5;
                Engine::textSub.drawGlyph(fnt, '<', x, pageChangeY);
            }
            if (itemPage < (optionCount - 1) / 2) {
                x = 256 - 15;
                Engine::textSub.drawGlyph(fnt, '>', x, pageChangeY);
            }
            for (int i = 0; i < 2; i++) {
                int itemIdx = (itemPage * 2) + i;
                if (itemIdx >= optionCount)
                    break;
                int item = globalSave.items[itemIdx];

                sprintf(buffer, "nitro:/data/items/name%d.txt", item);
                FILE* f = fopen(buffer, "rb");
                int len = str_len_file(f, '\n');
                fread(buffer, len + 1, 1, f);
                fclose(f);
                x = itemsX;
                if (i == optionSelected) {
                    listHeart.wx = (x - 12) << 8;
                    listHeart.wy = (y + 4) << 8;
                }
                for (char* pName = buffer; *pName != '\n'; pName++) {
                    Engine::textSub.drawGlyph(fnt, *pName, x, y);
                }
                y += itemSpacingY;
            }

            listHeart.setShown(true);
            itemExplainBox.setShown(true);

            // TODO: Make descriptions have multiple pages (ex. temmie armor)
            int itemIdx = itemPage * 2 + optionSelected;
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
                Engine::textSub.drawGlyph(fnt, *pName, x, y);
            }
        }
    } else {
        // CELL menu
        listHeart.setShown(true);
        itemExplainBox.setShown(false);
        for (optionCount = 0; globalSave.cell[optionCount] != 0; optionCount++);
        if (optionSelected > optionCount - 1)
            optionSelected = optionCount - 1;
        y = itemsY;
        for (int i = 0; i < optionCount; i++) {
            int cellOption = globalSave.cell[i];

            sprintf(buffer, "nitro:/data/cell/name%d.txt", cellOption);
            FILE* f = fopen(buffer, "rb");
            int len = str_len_file(f, '\n');
            fread(buffer, len + 1, 1, f);
            fclose(f);
            x = itemsX;
            if (i == optionSelected) {
                listHeart.wx = (x - 12) << 8;
                listHeart.wy = (y + 4) << 8;
            }
            for (char* pName = buffer; *pName != '\n'; pName++) {
                Engine::textSub.drawGlyph(fnt, *pName, x, y);
            }
            y += itemSpacingY;
        }
    }
}

void InGameMenu::update() {
    if (!shown)
        return;
    if (keysDown() & KEY_TOUCH) {
        touchPosition touch;
        touchRead(&touch);
        if (selectedMenu == MENU_ITEMS)
            processTouchItems(touch);
        else
            processTouchCell(touch);
    }
}

void InGameMenu::processTouchItems(touchPosition &touch) {
    if (touch.px > 140 && touch.px < 140 + 58 && touch.py > 35 && touch.py < 35 + 19) {
        if (selectedMenu != MENU_CELL && globalSave.flags[2] == 1) {
            selectedMenu = MENU_CELL;
            show(true);
        }
    }
    else if (touch.py > 35 + 19 && touch.py < itemsY + itemSpacingY * 2) {
        if (touch.px < itemsX + buttonWidth &&
            touch.px > itemsX) {
            int itemY = (touch.py - itemsY) / itemSpacingY;
            int itemIdx = itemY;
            if (itemIdx != optionSelected && itemPage * 2 + optionSelected < optionCount) {
                optionSelected = itemIdx;
                show(true);
            }
        } else if (touch.py > pageChangeY - 5 && touch.py < pageChangeY + 20) {
            if (touch.px < 15) {
                if (itemPage > 0) {
                    itemPage--;
                    show(true);
                }
            }
            else if (touch.px > 256 - 25) {
                if (itemPage < pageCount - 1) {
                    itemPage++;
                    show(true);
                }
            }
        }
    }
}

void InGameMenu::processTouchCell(touchPosition &touch) {
    if (touch.px > 53 && touch.px < 53 + 58 && touch.py > 35 && touch.py < 35 + 19) {
        if (selectedMenu != MENU_ITEMS) {
            selectedMenu = MENU_ITEMS;
            optionSelected = 0;
            itemPage = 0;
            show(true);
        }
    } else if (touch.px > itemsX && touch.py > itemsY && touch.py < itemsY + itemSpacingY * (optionCount + 1)) {
        int touchedOption = (touch.py - itemsY) / itemSpacingY;
        if (touchedOption != optionSelected) {
            optionSelected = touchedOption;
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
