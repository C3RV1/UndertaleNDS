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
    explicit ManagedSprite(Engine::AllocationMode alloc) : _spr(alloc) {}
    void load(ROOMSprite const& sprData,
              std::vector<std::shared_ptr<Engine::Texture>>& textures);
    void spawn(s8 textureId, s32 x, s32 y,
               std::vector<std::shared_ptr<Engine::Texture>>& textures);
    void update(bool isRoom);
    void draw(bool isRoom);
    Engine::Sprite _spr;

    u8 _interactAction = 0;
    u16 _cutsceneId = 0;
    u32 _distanceSquared = 0;
    int _closeAnim = 0;
    int _animationId = 0;
private:
    void free_();
    std::shared_ptr<Engine::Texture> _texture = nullptr;
};

#endif //UNDERTALE_MANAGED_SPRITE_HPP
