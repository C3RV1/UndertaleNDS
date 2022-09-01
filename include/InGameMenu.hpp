//
// Created by cervi on 01/09/2022.
//

#ifndef LAYTON_INGAMEMENU_HPP
#define LAYTON_INGAMEMENU_HPP

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
    InGameMenu() : selectedMenuHeart(Engine::AllocatedOAM), listHeart(Engine::AllocatedOAM) {}
    void load();
    void unload();
    void show();
    void hide();
    void update();
private:
    bool shown = false;
    Engine::Font fnt;
    Engine::Background bg;
    Engine::Sprite littleHeart;
    Engine::SpriteManager selectedMenuHeart;
    Engine::SpriteManager listHeart;
    SelectedMenu selectedMenu = MENU_ITEMS;
};

extern InGameMenu globalInGameMenu;

#endif //LAYTON_INGAMEMENU_HPP
