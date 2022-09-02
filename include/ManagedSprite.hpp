//
// Created by cervi on 28/08/2022.
//

#ifndef LAYTON_MANAGEDSPRITE_HPP
#define LAYTON_MANAGEDSPRITE_HPP

class ManagedSprite;

#include "Engine/Texture.hpp"
#include "Engine/Sprite3DManager.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Room/Camera.hpp"

class ManagedSprite {
public:
    explicit ManagedSprite(Engine::AllocationMode alloc) : spriteManager(alloc) {}
    void load(ROOMSprite* sprData);
    void spawn(char* path, int32_t x, int32_t y, int32_t layer);
    void draw(bool isRoom);
    void free_();
    Engine::Sprite spriteManager;
private:
    Engine::Texture spr;
    int animationId = -1;
};

#endif //LAYTON_MANAGEDSPRITE_HPP
