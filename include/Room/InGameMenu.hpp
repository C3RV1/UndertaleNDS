//
// Created by cervi on 01/09/2022.
//

#ifndef UNDERTALE_INGAMEMENU_HPP
#define UNDERTALE_INGAMEMENU_HPP

#include "Font.hpp"
#include "Save.hpp"
#include "Background.hpp"
#include "Engine.hpp"

enum SelectedMenu {
    MENU_ITEMS,
    MENU_CELL
};

class InGameMenu {
public:
    InGameMenu() :
    selectedMenuHeart(Engine::AllocatedOAM),
    listHeart(Engine::AllocatedOAM),
    itemExplainBox(Engine::AllocatedOAM){}
    void load();
    void unload();
    void show(bool update);
    void hide();
    void update();
private:
    const int nameX = 18, nameY = 10;
    const int hpX = 120, hpY = 10;
    const int lvX = 226, lvY = 10;
    const int expX = 226, expY = 22;
    const int selectedMenuX = 59 << 8, selectedMenuY = 40 << 8;
    const int selectedMenuSeparation = (146 - 59) << 8;
    const int itemsX = 58, itemsY = 58, itemSpacingY = 15;
    const int pageChangeY = itemsY + itemSpacingY - itemSpacingY / 2;
    const int buttonWidth = 90;

    bool shown = false;
    Engine::Font fnt;
    Engine::Background bg;
    Engine::Texture littleHeart;
    Engine::Texture itemExplain;
    Engine::Sprite selectedMenuHeart;
    Engine::Sprite listHeart;
    Engine::Sprite itemExplainBox;
    SelectedMenu selectedMenu = MENU_ITEMS;
    uint8_t itemCount = 0;
    uint8_t pageCount = 0;
    uint8_t itemSelected = 0;
    uint8_t itemPage = 0;
};

extern InGameMenu globalInGameMenu;

#endif //UNDERTALE_INGAMEMENU_HPP
