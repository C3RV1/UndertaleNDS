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
    if (shown)
        return;

    const int nameX = 18, nameY = 10;
    const int hpX = 120, hpY = 10;
    const int lvX = 226, lvY = 10;
    const int expX = 226, expY = 22;
    const int selectedMenuX = 59 << 8, selectedMenuY = 40 << 8;
    const int selectedMenuSeparation = (146 - 59) << 8;

    shown = true;
    Engine::loadBgTextSub(bg);
    Engine::textSub.clear();
    Engine::textSub.setCurrentColor(15);
    selectedMenuHeart.setShown(true);
    selectedMenuHeart.wx = selectedMenuX + selectedMenuSeparation * selectedMenu;
    selectedMenuHeart.wy = selectedMenuY;
    // listHeart.setShown(true);

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
}

InGameMenu globalInGameMenu;
