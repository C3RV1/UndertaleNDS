//
// Created by cervi on 30/08/2022.
//
#include "Sprite.hpp"

namespace Engine {
    Sprite::Sprite(Engine::AllocationMode allocMode_) {
        allocMode = allocMode_;
    }

    void Sprite::setSpriteAnim(int animId) {
        if (!loaded)
            return;
        if (animId >= texture->getAnimCount())
            return;
        if (currentAnimation == animId)
            return;
        currentAnimation = animId;
        currentAnimationFrame = 0;
        currentAnimationTimer = texture->getAnimations()[animId].frames[0].duration;
        currentFrame = texture->getAnimations()[animId].frames[0].frame;
    }

    void Sprite::loadTexture(Engine::Texture &sprite_) {
        if (!sprite_.getLoaded())
            return;
        if (&sprite_ == texture)
            return;
        push();

        texture = &sprite_;
        loaded = true;
        currentFrame = 0;
        currentAnimation = -1;

        int animId = nameToAnimId("gfx");  // default animation
        if (animId != -1)
            setSpriteAnim(animId);
        pop();
    }

    void Sprite::tick() {
        if (!loaded)
            return;
        if (currentAnimation >= 0) {
            CSPRAnimation* current = &texture->getAnimations()[currentAnimation];
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

        x = wx - cam_x;
        y = wy - cam_y;
        if (currentAnimation >= 0) {
            CSPRAnimation* current = &texture->getAnimations()[currentAnimation];
            CSPRAnimFrame* frameInfo = &current->frames[currentAnimationFrame];
            x += frameInfo->drawOffX << 8;
            y += frameInfo->drawOffY << 8;
        }
        x *= cam_scale_x;
        x >>= 8;
        y *= cam_scale_y;
        y >>= 8;
        scale_x = (cam_scale_x * w_scale_x) >> 8;
        scale_y = (cam_scale_y * w_scale_y) >> 8;
    }

    void Sprite::setShown(bool shown_) {
        if (!loaded)
            return;
        if (shown_ == shown)
            return;
        shown = shown_;
        if (shown) {
            if (memory.allocated != NoAlloc)
                return;
            if (allocMode == Allocated3D)
                main3dSpr.loadSprite(*this);
            else if (allocMode == AllocatedOAM)
                OAMManagerSub.loadSprite(*this);
        } else {
            if (memory.allocated == Allocated3D)
                main3dSpr.freeSprite(*this);
            else if (memory.allocated == AllocatedOAM)
                OAMManagerSub.freeSprite(*this);
        }
    }

    int Sprite::nameToAnimId(const char *animName) const {
        if (!loaded)
            return -1;
        for (int i = 0; i < texture->getAnimCount(); i++) {
            if (strcmp(animName, texture->getAnimations()[i].name) == 0) {
                return i;
            }
        }
        return -1;
    }

    void Sprite::push() {
        pushed = shown;
        setShown(false);
    }

    void Sprite::pop() {
        setShown(pushed);
    }
}
