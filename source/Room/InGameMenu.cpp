//
// Created by cervi on 01/09/2022.
//
#include "Room/InGameMenu.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"

void InGameMenu::load() {
    char buffer[100];
    FILE *f;
    f = fopen("nitro:/fnt/fnt_maintext.font.cfnt", "rb");
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

    f = fopen("nitro:/spr/spr_heartsmall.cspr", "rb");
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

    f = fopen("nitro:/spr/ingame_menu/item_explain.cspr", "rb");
    if (f) {
        int sprLoad = itemExplain.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading item explain spr: %d", sprLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening item explain spr");
    }
    fclose(f);

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

    shown = true;
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
            for (itemCount = 0; globalSave.items[itemCount] != 0; itemCount++);
            pageCount = ((itemCount - 1) / 2) + 1;
            if (itemPage > pageCount - 1)
                itemPage = pageCount - 1;
            if (itemSelected > itemCount - itemPage * 2 - 1)
                itemSelected = itemCount - itemPage * 2 - 1;
            if (itemPage > 0) {
                x = 5;
                Engine::textSub.drawGlyph(fnt, '<', x, pageChangeY);
            }
            if (itemPage < (itemCount - 1) / 2) {
                x = 256 - 15;
                Engine::textSub.drawGlyph(fnt, '>', x, pageChangeY);
            }
            for (int i = 0; i < 2; i++) {
                int itemIdx = (itemPage * 2) + i;
                if (itemIdx >= itemCount)
                    break;
                int item = globalSave.items[itemIdx];

                sprintf(buffer, "nitro:/data/items/name%d.txt", item);
                FILE* f = fopen(buffer, "rb");
                int len = strlen_file(f, '\n');
                fread(buffer, len + 1, 1, f);
                fclose(f);
                x = itemsX;
                if (i == itemSelected) {
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
            int itemIdx = itemPage * 2 + itemSelected;
            int item = globalSave.items[itemIdx];
            sprintf(buffer, "nitro:/data/items/desc%d.txt", item);
            FILE* f = fopen(buffer, "rb");
            int len = strlen_file(f, '\0');
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
        listHeart.setShown(false);
        itemExplainBox.setShown(false);
    }
}

void InGameMenu::update() {
    if (!shown)
        return;
    if (keysDown() & KEY_TOUCH) {
        touchPosition touch;
        touchRead(&touch);
        if (touch.py > 35 && touch.py < 35 + 19) {
            if (touch.px > 53 && touch.px < 53 + 58) {
                if (selectedMenu != MENU_ITEMS) {
                    selectedMenu = MENU_ITEMS;
                    itemSelected = 0;
                    itemPage = 0;
                    show(true);
                }
            } else if (touch.px > 140 && touch.px < 140 + 58) {
                if (selectedMenu != MENU_CELL) {
                    selectedMenu = MENU_CELL;
                    show(true);
                }
            }
        } else if (touch.py > 35 + 19 && touch.py < itemsY + itemSpacingY * 2) {
            if (selectedMenu == MENU_ITEMS) {
                if (touch.px < itemsX + buttonWidth &&
                    touch.px > itemsX) {
                    int itemY = (touch.py - itemsY) / itemSpacingY;
                    int itemIdx = itemY;
                    if (itemIdx != itemSelected && itemPage * 2 + itemSelected < itemCount) {
                        itemSelected = itemIdx;
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
    }
}

InGameMenu globalInGameMenu;
