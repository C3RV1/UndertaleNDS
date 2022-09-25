//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_MANAGEDSPRITE_HPP
#define UNDERTALE_MANAGEDSPRITE_HPP

class ManagedSprite;

#include "Engine/Sprite.hpp"
#include "Formats/ROOM_FILE.hpp"

class ManagedSprite {
public:
    explicit ManagedSprite(Engine::AllocationMode alloc) : spriteManager(alloc) {}
    void load(ROOMSprite* sprData, uint8_t textureCount, Engine::Texture** textures);
    void spawn(uint8_t textureId, int32_t x, int32_t y, int32_t layer,
               uint8_t textureCount, Engine::Texture** textures);
    void draw(bool isRoom);
    void free_();
    Engine::Sprite spriteManager;

    uint8_t interactAction = 0;
    uint16_t cutsceneId = 0;
private:
    Engine::Texture* texture = nullptr;
    int animationId = -1;
};

#endif //UNDERTALE_MANAGEDSPRITE_HPP
