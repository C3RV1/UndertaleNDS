//
// Created by cervi on 28/08/2022.
//

#include "ManagedSprite.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"
#include "Engine/math.hpp"

void ManagedSprite::load(ROOMSprite *sprData, u8 textureCount,
                         Engine::Texture** textures) {
    if (sprData->textureId < textureCount) {
        texture = textures[sprData->textureId];
        spriteManager.loadTexture(*texture);
    }
    animationId = spriteManager.nameToAnimId(sprData->animation);
    spriteManager.wx = sprData->x << 8;
    spriteManager.wy = sprData->y << 8;
    spriteManager.setSpriteAnim(animationId);

    spriteManager.setShown(true);

    interactAction = sprData->interactAction;
    if (interactAction == 1)
        cutsceneId = sprData->cutsceneId;
    else if (interactAction == 2) {
        distanceSquared = sprData->distance * sprData->distance;
        closeAnim = spriteManager.nameToAnimId(sprData->closeAnim);
    }
}

void ManagedSprite::spawn(u8 textureId, s32 x, s32 y,
                          u8 textureCount, Engine::Texture** textures) {
    if (textureId < textureCount) {
        texture = textures[textureId];
        spriteManager.loadTexture(*texture);
    }
    spriteManager.wx = x;
    spriteManager.wy = y;

    spriteManager.setShown(true);
}

void ManagedSprite::draw(bool isRoom) {
    if (isRoom) {
        spriteManager.cam_x = globalCamera.pos.wx;
        spriteManager.cam_y = globalCamera.pos.wy;
        spriteManager.cam_scale_x = globalCamera.pos.w_scale_x;
        spriteManager.cam_scale_y = globalCamera.pos.w_scale_y;
        spriteManager.layer = spriteManager.wy >> 8;
    }
}

void ManagedSprite::update(bool isRoom) {
    if (isRoom && interactAction == 2) {
        if (spriteManager.texture == nullptr)
            return;
        if (globalPlayer->spriteManager.texture == nullptr)
            return;
        u16 width = spriteManager.texture->getWidth();
        u16 height = spriteManager.texture->getHeight();
        u16 pw = globalPlayer->spriteManager.texture->getWidth();
        u16 ph = globalPlayer->spriteManager.texture->getHeight();
        u32 distance = distSquared_fp(spriteManager.wx + width / 2,
                                      spriteManager.wy + height / 2,
                                      globalPlayer->spriteManager.wx + pw / 2,
                                      globalPlayer->spriteManager.wy + ph / 2);
        if (distance >> 8 < distanceSquared)
            spriteManager.setSpriteAnim(closeAnim);
        else
            spriteManager.setSpriteAnim(animationId);
    }
}

void ManagedSprite::free_() {
    spriteManager.setShown(false);
}