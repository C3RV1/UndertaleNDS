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
    RoomSprite() : spriteManager(Engine::Allocated3D) {}
    void load(ROOMSprite* sprData);
    void hide();
    void draw();
    void free_();
    Engine::SpriteManager spriteManager;
private:
    int32_t x = 0, y = 0; // 23 bit integer, 8 bit fraction
    Engine::Sprite spr;
    int animationId = -1;
};

#endif //LAYTON_ROOMSPRITE_HPP
