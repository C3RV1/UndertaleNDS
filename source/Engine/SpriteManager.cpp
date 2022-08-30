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
        x = wx - cam_x;
        y = wy - cam_y;
        x *= cam_scale_x;
        x >>= 8;
        y *= cam_scale_y;
        y >>= 8;
        scale_x = (cam_scale_x * wscale_x) >> 8;
        scale_y = (cam_scale_y * wscale_y) >> 8;

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
