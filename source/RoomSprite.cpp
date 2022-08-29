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

    animationId = spr.nameToAnimId(sprData->animation);
    spriteManager.loadSprite(spr);
    spriteManager.wx = sprData->x << 8;
    spriteManager.wy = sprData->y << 8;
    spriteManager.setSpriteAnim(animationId);

    show();
}

void RoomSprite::show() {
    char buffer[100];
    int loadSprite = Engine::main3dSpr.loadSprite(spriteManager);
    if (loadSprite != 0) {
        sprintf(buffer, "Error showing room sprite: %d", loadSprite);
        nocashMessage(buffer);
        return;
    }
}

void RoomSprite::hide() {
    Engine::main3dSpr.freeSprite(spriteManager);
}

void RoomSprite::draw() {
    spriteManager.x = spriteManager.wx - globalCamera.x;
    spriteManager.y = spriteManager.wy - globalCamera.y;
    spriteManager.layer = 1;
}

void RoomSprite::free_() {
    hide();
    spr.free_();
}