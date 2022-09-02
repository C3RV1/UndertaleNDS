//
// Created by cervi on 28/08/2022.
//

#include "ManagedSprite.hpp"

void ManagedSprite::load(ROOMSprite *sprData) {
    char buffer[100];
    FILE *f = fopen(sprData->spritePath, "rb");
    if (f) {
        int sprLoad = spr.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading RoomSprite %s: %d",
                    sprData->spritePath, sprLoad);
            nocashMessage(buffer);
        }
    } else {
        sprintf(buffer, "Error opening RoomSprite %s", sprData->spritePath);
        nocashMessage(buffer);
    }
    fclose(f);

    spriteManager.loadSprite(spr);
    animationId = spriteManager.nameToAnimId(sprData->animation);
    spriteManager.wx = sprData->x << 8;
    spriteManager.wy = sprData->y << 8;
    spriteManager.setSpriteAnim(animationId);

    spriteManager.setShown(true);
}

void ManagedSprite::spawn(char *path, int32_t x, int32_t y) {
    char buffer[100];
    FILE *f = fopen(path, "rb");
    if (f) {
        int sprLoad = spr.loadCSPR(f);
        if (sprLoad != 0) {
            sprintf(buffer, "Error loading RoomSprite %s: %d",
                    path, sprLoad);
            nocashMessage(buffer);
        }
    } else {
        sprintf(buffer, "Error opening RoomSprite %s", path);
        nocashMessage(buffer);
    }
    fclose(f);

    spriteManager.loadSprite(spr);
    spriteManager.wx = x;
    spriteManager.wy = y;

    spriteManager.setShown(true);
}

void ManagedSprite::draw() {
    spriteManager.cam_x = globalCamera.pos.wx;
    spriteManager.cam_y = globalCamera.pos.wy;
    spriteManager.cam_scale_x = globalCamera.pos.wscale_x;
    spriteManager.cam_scale_y = globalCamera.pos.wscale_y;
    spriteManager.layer = 1;
}

void ManagedSprite::free_() {
    spriteManager.setShown(false);
    spr.free_();
}