//
// Created by cervi on 28/08/2022.
//

#include "ManagedSprite.hpp"

void ManagedSprite::load(ROOMSprite *sprData, uint8_t textureCount,
                         Engine::Texture** textures) {
    if (sprData->textureId < textureCount) {
        texture = textures[sprData->textureId];
        spriteManager.loadTexture(*texture);
    }
    animationId = spriteManager.nameToAnimId(sprData->animation);
    spriteManager.wx = sprData->x << 8;
    spriteManager.wy = sprData->y << 8;
    spriteManager.layer = sprData->layer;
    spriteManager.setSpriteAnim(animationId);

    spriteManager.setShown(true);
}

void ManagedSprite::spawn(uint8_t textureId, int32_t x, int32_t y, int32_t layer,
                          uint8_t textureCount, Engine::Texture** textures) {
    if (textureId < textureCount) {
        texture = textures[textureId];
        spriteManager.loadTexture(*texture);
    }
    spriteManager.wx = x;
    spriteManager.wy = y;
    spriteManager.layer = layer;

    spriteManager.setShown(true);
}

void ManagedSprite::draw(bool isRoom) {
    if (isRoom) {
        spriteManager.cam_x = globalCamera.pos.wx;
        spriteManager.cam_y = globalCamera.pos.wy;
        spriteManager.cam_scale_x = globalCamera.pos.wscale_x;
        spriteManager.cam_scale_y = globalCamera.pos.wscale_y;
    }
}

void ManagedSprite::free_() {
    spriteManager.setShown(false);
}