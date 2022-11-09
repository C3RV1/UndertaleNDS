//
// Created by cervi on 01/09/2022.
//

#ifndef UNDERTALE_INGAME_MENU_HPP
#define UNDERTALE_INGAME_MENU_HPP

#include "Engine/Sprite.hpp"
#include "Engine/Font.hpp"
#include "Engine/Background.hpp"

enum SelectedMenu {
    MENU_ITEMS,
    MENU_CELL
};

class InGameMenu {
public:
    InGameMenu() :
            _selectedMenuHeartSpr(Engine::AllocatedOAM),
            _listHeartSpr(Engine::AllocatedOAM),
            _itemExplainBoxSpr(Engine::AllocatedOAM){}
    void load();
    void unload();
    void show(bool update);
    void processTouchItems(touchPosition& touch);
    void processTouchCell(touchPosition& touch);
    void hide();
    void update();
private:
    const int kNameX = 18, kNameY = 10;
    const int kHpX = 120, kHpY = 10;
    const int kLvX = 226, kLvY = 10;
    const int kExpX = 226, kExpY = 22;
    const int kSelectedMenuX = 59 << 8, kSelectedMenuY = 40 << 8;
    const int kSelectedMenuSeparation = (146 - 59) << 8;
    const int kItemsX = 58, kItemsY = 58, kItemSpacingY = 15;
    const int kPageChangeY = kItemsY + kItemSpacingY - kItemSpacingY / 2;
    const int kButtonWidth = 90;

    bool _shown = false;
    Engine::Font _fnt;
    Engine::Background _bg;
    Engine::Texture _littleHeartTex;
    Engine::Texture _itemExplainTex;
    Engine::Sprite _selectedMenuHeartSpr;
    Engine::Sprite _listHeartSpr;
    Engine::Sprite _itemExplainBoxSpr;
    SelectedMenu _selectedMenu = MENU_ITEMS;
    u8 _optionCount = 0;
    u8 _pageCount = 0;
    u8 _optionSelected = 0;
    u8 _itemPage = 0;

    bool _bgLoadedCell = false;
};

extern InGameMenu globalInGameMenu;

#endif //UNDERTALE_INGAME_MENU_HPP
