//
// Created by cervi on 28/08/2022.
//

#include "RoomSprite.hpp"

void RoomSprite::load(ROOMSprite *sprData) {
    char buffer[100];
    FILE *f = fopen(sprData->spritePath, "rb");
    if (f) {
        nocashMessage("precspr");
        int sprLoad = spr.loadCSPR(f);
        nocashMessage("postcspr");
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

void RoomSprite::draw() {
    spriteManager.cam_x = globalCamera.pos.wx;
    spriteManager.cam_y = globalCamera.pos.wy;
    spriteManager.cam_scale_x = globalCamera.pos.wscale_x;
    spriteManager.cam_scale_y = globalCamera.pos.wscale_y;
    spriteManager.layer = 1;
}

void RoomSprite::free_() {
    spriteManager.setShown(false);
    spr.free_();
}