//
// Created by cervi on 27/08/2022.
//

#include "Player.hpp"

Player::Player() {
    char buffer[100];
    FILE *f = fopen("nitro:/spr/spr_f_mainchara.cspr", "rb");
    if (f) {
        int playerLoad = playerSpr.loadCSPR(f);
        if (playerLoad != 0) {
            sprintf(buffer, "Error loading player sprite: %d", playerLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening player sprite");
    }
    fclose(f);
}

void Player::showPlayer() {
    if (sprManager != nullptr)
        return;
    char buffer[100];
    int loadSprite = Engine::main3dSpr.loadSprite(playerSpr, sprManager);
    if (loadSprite != 0) {
        sprintf(buffer, "Error showing player: %d", loadSprite);
        nocashMessage(buffer);
        return;
    }
    sprControl = Engine::main3dSpr.getSpriteControl(sprManager);
}

void Player::hidePlayer() {
    if (sprManager == nullptr)
        return;
    Engine::main3dSpr.freeSprite(sprManager);
    sprControl = nullptr;
}

void Player::update(Room*& room, Camera& cam) {
    int32_t prevX = x, prevY = y;
    int moveDirection;
    bool setAnim = true;
    if (keysHeld() & KEY_DOWN) {
        y += MOVE_SPEED;
        moveDirection = 1;
        if (currentAnimation == 5)
            setAnim = false;
    }
    if (keysHeld() & KEY_UP) {
        y -= MOVE_SPEED;
        moveDirection = 0;
        if (currentAnimation == 4)
            setAnim = false;
    }
    if (keysHeld() & KEY_RIGHT) {
        x += MOVE_SPEED;
        moveDirection = 3;
        if (currentAnimation == 7)
            setAnim = false;
    }
    if (keysHeld() & KEY_LEFT) {
        x -= MOVE_SPEED;
        moveDirection = 2;
        if (currentAnimation == 6)
            setAnim = false;
    }

    if (prevX != x || prevY != y) {
        check_exits(room, cam);
        int x2 = x, y2 = y;
        if (check_collisions(room)) {
            x = prevX;
            y = y2;
            if (check_collisions(room)) {
                x = x2;
                y = prevY;
                if (check_collisions(room)) {
                    x = prevX;
                    y = prevY;
                }
            }
        }
    }

    if (x == prevX && prevY == y) {
        if (currentAnimation > 3) {
            currentAnimation -= 4;
            animFrame = 0;
        }
    } else {
        if (setAnim) {
            currentAnimation = moveDirection + 4;
            animFrame = 0;
        }
    }
}

void Player::check_exits(Room*& room, Camera& cam) {
    uint8_t width, height;
    room->bg.getSize(width, height);
    if (x < 0) {
        x = 0;
        if (room->exitLeft != nullptr) {
            x = room->exitLeft->spawnX << 8;
            y = room->exitLeft->spawnY << 8;
            loadNewRoom(room, cam, *this, room->exitLeft->roomId);
        }
    }
    else if ((x >> 8) + 19 > width * 8) {
        x = (width * 8 - 19) << 8;
        if (room->exitRight != nullptr) {
            x = room->exitRight->spawnX << 8;
            y = room->exitRight->spawnY << 8;
            loadNewRoom(room, cam, *this, room->exitRight->roomId);
        }
    }
    if (y < 0) {
        y = 0;
        if (room->exitTop != nullptr) {
            x = room->exitTop->spawnX << 8;
            y = room->exitTop->spawnY << 8;
            loadNewRoom(room, cam, *this, room->exitTop->roomId);
        }
    }
    else if ((y >> 8) + 29 > height * 8) {
        y = (height * 8 - 29) << 8;
        if (room->exitBtm != nullptr) {
            x = room->exitBtm->spawnX << 8;
            y = room->exitBtm->spawnY << 8;
            loadNewRoom(room, cam, *this, room->exitBtm->roomId);
        }
    }

    for (int i = 0; i < room->rectExitCount; i++) {
        RoomExit* rectExit = room->rectExits[i];
        if (collidesRect(x >> 8, (y >> 8) + 20, 19, 9,
                         rectExit->x, rectExit->y,
                         rectExit->w, rectExit->h)) {
            x = rectExit->spawnX << 8;
            y = rectExit->spawnY << 8;
            loadNewRoom(room, cam, *this, rectExit->roomId);
        }
    }
}

bool Player::check_collisions(Room*& room) const {
    for (int i = 0; i < room->roomData.roomColliders.colliderCount; i++) {
        RoomCollider* collider = &room->roomData.roomColliders.roomColliders[i];
        if (collidesRect(x >> 8, (y >> 8) + 20, 19, 9,
                         collider->x, collider->y,
                         collider->w, collider->h)) {
            return true;
        }
    }
    return false;
}

void Player::draw(Camera &cam) {
    int playerFrame = 0;
    sprControl->x = (x - cam.x) >> 8;
    sprControl->y = (y - cam.y) >> 8;
    if (currentAnimation > 3) {
        animFrame++;
        switch (currentAnimation) {
            case 4:
                animFrame %= 4 * FRAME_TIME;
                playerFrame = 8 + animFrame / FRAME_TIME;
                break;
            case 5:
                animFrame %= 4 * FRAME_TIME;
                playerFrame = 0 + animFrame / FRAME_TIME;
                break;
            case 6:
                animFrame %= 2 * FRAME_TIME;
                playerFrame = 4 + animFrame / FRAME_TIME;
                break;
            case 7:
                animFrame %= 2 * FRAME_TIME;
                playerFrame = 6 + animFrame / FRAME_TIME;
                break;
        }
    } else {
        switch (currentAnimation) {
            case 0:
                playerFrame = 8;
                break;
            case 1:
                playerFrame = 0;
                break;
            case 2:
                playerFrame = 4;
                break;
            case 3:
                playerFrame = 6;
                break;
        }
    }
    Engine::main3dSpr.loadSpriteFrame(sprManager, playerFrame);
}
