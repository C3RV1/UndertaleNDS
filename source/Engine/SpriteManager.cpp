//
// Created by cervi on 30/08/2022.
//
#include "SpriteManager.hpp"

namespace Engine {
    void SpriteManager::setSpriteAnim(int animId) {
        if (!loaded)
            return;
        if (animId >= sprite->getAnimCount())
            return;
        if (currentAnimation == animId)
            return;
        currentAnimation = animId;
        currentAnimationFrame = 0;
        currentAnimationTimer = sprite->getAnims()[animId].frames[0].duration;
        currentFrame = sprite->getAnims()[animId].frames[0].frame;
    }

    void SpriteManager::loadSprite(Engine::Sprite &sprite_) {
        if (!sprite_.getLoaded())
            return;

        currentFrame = -1;
        sprite = &sprite_;
        loaded = true;
    }

    void SpriteManager::copyNoMemory(Engine::SpriteManager &spriteManager) {
        if (sprite != spriteManager.sprite)
            return;
        x = spriteManager.x; y = spriteManager.y;
        layer = spriteManager.layer;
        currentFrame = spriteManager.currentFrame;
        currentAnimation = spriteManager.currentAnimation;
        currentAnimationTimer = spriteManager.currentAnimationTimer;
        currentAnimationFrame = spriteManager.currentAnimationFrame;
    }
}
