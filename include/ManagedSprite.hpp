//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_MANAGED_SPRITE_HPP
#define UNDERTALE_MANAGED_SPRITE_HPP

class ManagedSprite;

#include "Engine/Sprite.hpp"
#include "Formats/ROOM_FILE.hpp"

class ManagedSprite {
public:
    explicit ManagedSprite(Engine::AllocationMode alloc) : spriteManager(alloc) {}
    void load(ROOMSprite* sprData, u8 textureCount, Engine::Texture** textures);
    void spawn(u8 textureId, s32 x, s32 y, s32 layer,
               u8 textureCount, Engine::Texture** textures);
    void draw(bool isRoom);
    void free_();
    Engine::Sprite spriteManager;

    u8 interactAction = 0;
    u16 cutsceneId = 0;
private:
    Engine::Texture* texture = nullptr;
    int animationId = -1;
};

#endif //UNDERTALE_MANAGED_SPRITE_HPP
