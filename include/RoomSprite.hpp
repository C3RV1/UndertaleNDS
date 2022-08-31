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
    void spawn(char* path, int32_t x, int32_t y);
    void draw();
    void free_();
    Engine::SpriteManager spriteManager;
private:
    Engine::Sprite spr;
    int animationId = -1;
};

#endif //LAYTON_ROOMSPRITE_HPP
