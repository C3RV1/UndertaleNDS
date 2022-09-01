//
// Created by cervi on 01/09/2022.
//
#include "InGameMenu.hpp"

void InGameMenu::load() {
    char buffer[100];
    FILE *f;
    f = fopen("nitro:/fnt/fnt_main.font.cfnt", "rb");
    if (f) {
        int fontLoad = fnt.loadCFNT(f);
        if (fontLoad != 0) {
            sprintf(buffer, "Error loading font: %d", fontLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening font file");
    }
    fclose(f);

    f = fopen("nitro:/bg/ingame_menu/bg.cbgf", "rb");
    if (f) {
        int bgLoad = bg.loadCBGF(f);
        if (bgLoad != 0) {
            sprintf(buffer, "Error loading in game menu bg: %d", bgLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening in game menu bg");
    }
    fclose(f);

    f = fopen("nitro:/spr/ingame_menu/spr_heartsmall.cspr", "rb");
    if (f) {
        int sprLoad = littleHeart.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading little heart spr: %d", sprLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening little heart spr");
    }
    fclose(f);

    selectedMenuHeart.loadSprite(littleHeart);
    listHeart.loadSprite(littleHeart);

    show();
}

void InGameMenu::hide() {
    if (!shown)
        return;
    shown = false;
    Engine::textSub.clear();
    Engine::clearSub();
    selectedMenuHeart.setShown(false);
}

void InGameMenu::show() {
    // TODO: Make Font so we don't have to render all text again
    //       when we change the selected menu (consumes a lot)
    //       Or maybe optimize? Or maybe don't do anything, it
    //       only runs for a single frame so who cares?

    const int nameX = 18, nameY = 10;
    const int hpX = 120, hpY = 10;
    const int lvX = 226, lvY = 10;
    const int expX = 226, expY = 22;
    const int selectedMenuX = 59 << 8, selectedMenuY = 40 << 8;
    const int selectedMenuSeparation = (146 - 59) << 8;
    const int itemsX = 34, itemsY = 60, itemSpacing = 15;

    shown = true;
    Engine::loadBgTextSub(bg);
    Engine::textSub.clear();
    Engine::textSub.setCurrentColor(15);
    selectedMenuHeart.setShown(true);
    selectedMenuHeart.wx = selectedMenuX + selectedMenuSeparation * selectedMenu;
    selectedMenuHeart.wy = selectedMenuY;

    char buffer[100];
    int x = nameX, y = nameY;
    for (char* pName = saveGlobal.name; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d/%d", saveGlobal.hp, saveGlobal.maxHp);
    x = hpX, y = hpY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", saveGlobal.lv);
    x = lvX, y = lvY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    sprintf(buffer, "%d", saveGlobal.exp);
    x = expX, y = expY;
    for (char* pName = buffer; *pName != 0; pName++) {
        Engine::textSub.drawGlyph(fnt, *pName, x, y);
    }

    if (selectedMenu == MENU_ITEMS) {
        if (saveGlobal.items[0] == 0) {
            listHeart.setShown(false);
        } else {
            y = itemsY;
            for (itemCount = 0; saveGlobal.items[itemCount] != 0; itemCount++) {
                sprintf(buffer, "nitro:/data/items/name%d.txt", saveGlobal.items[itemCount]);
                FILE* f = fopen(buffer, "rb");
                int len = strlen_file(f, '\n');
                fread(buffer, len + 1, 1, f);
                x = itemsX;
                for (char* pName = buffer; *pName != '\n'; pName++) {
                    Engine::textSub.drawGlyph(fnt, *pName, x, y);
                }
                y += itemSpacing;
            }
            if (itemSelected >= itemCount)
                itemSelected = 0;

            listHeart.setShown(true);
            listHeart.wx = 22 << 8;
            listHeart.wy = (62 + itemSpacing * itemSelected) << 8;
        }
    }
}

void InGameMenu::update() {
    if (!shown)
        return;
    if (keysDown() & KEY_TOUCH) {
        touchPosition touch;
        touchRead(&touch);
        if (touch.py >= 35 && touch.py <= 35 + 19) {
            if (touch.px >= 53 && touch.px <= 53 + 58) {
                if (selectedMenu != MENU_ITEMS) {
                    selectedMenu = MENU_ITEMS;
                    itemSelected = 0;
                    show();
                }
            } else if (touch.px >= 140 && touch.px <= 140 + 58) {
                if (selectedMenu != MENU_CELL) {
                    selectedMenu = MENU_CELL;
                    show();
                }
            }
        } else {
            if (selectedMenu == MENU_ITEMS) {
                if (touch.px <= 110 && touch.px >= 16) {

                }
            }
        }
    }
}

InGameMenu globalInGameMenu;
