//
// Created by cervi on 28/08/2022.
//

#ifndef LAYTON_ROOMSPRITE_HPP
#define LAYTON_ROOMSPRITE_HPP

class RoomSprite;

#include "Engine/Sprite.hpp"
#include "Engine/Sprite3DManager.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Camera.hpp"

class RoomSprite {
public:
    void load(ROOMSprite* sprData);
    void show();
    void hide();
    void draw();
    void free_();
    Engine::SpriteManager spriteManager;
private:
    int32_t x, y; // 23 bit integer, 8 bit fraction
    Engine::Sprite spr;
    int animationId;
};

#endif //LAYTON_ROOMSPRITE_HPP
