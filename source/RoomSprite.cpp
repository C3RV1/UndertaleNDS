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
    sprManager = nullptr;
    sprControl = nullptr;
    x = sprData->x << 8;
    y = sprData->y << 8;

    show();
}

void RoomSprite::show() {
    if (sprManager != nullptr)
        return;
    int loadSpriteRes = Engine::main3dSpr.loadSprite(spr, sprManager);
    if (loadSpriteRes == 0)
        sprControl = Engine::main3dSpr.getSpriteControl(sprManager);
}

void RoomSprite::hide() {
    if (sprManager == nullptr)
        return;
    Engine::main3dSpr.freeSprite(sprManager);
    sprControl = nullptr;
}

void RoomSprite::draw(Camera &cam) {
    if (sprControl == nullptr)
        return;
    sprControl->x = (x - cam.x) >> 8;
    sprControl->y = (y - cam.y) >> 8;
    sprControl->layer = 1;
    Engine::main3dSpr.setSpriteAnim(sprManager, animationId);
}

void RoomSprite::free_() {
    hide();
    spr.free_();
}