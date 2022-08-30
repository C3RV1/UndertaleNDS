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

    void SpriteManager::tick() {
        if (currentAnimation > 0) {
            CSPRAnimation* current = &sprite->getAnims()[currentAnimation];
            if (current->frames[currentAnimationFrame].duration != 0) {
                currentAnimationTimer--;
                if (currentAnimationTimer == 0) {
                    currentAnimationFrame++;
                    currentAnimationFrame %= current->frameCount;
                    currentFrame = current->frames[currentAnimationFrame].frame;
                    currentAnimationTimer = current->frames[currentAnimationFrame].duration;
                }
            }
        }
    }
}
