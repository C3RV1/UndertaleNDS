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

    upIdleId = playerSpr.nameToAnimId("upIdle");
    downIdleId = playerSpr.nameToAnimId("downIdle");
    leftIdleId = playerSpr.nameToAnimId("leftIdle");
    rightIdleId = playerSpr.nameToAnimId("rightIdle");
    upMoveId = playerSpr.nameToAnimId("upMove");
    downMoveId = playerSpr.nameToAnimId("downMove");
    leftMoveId = playerSpr.nameToAnimId("leftMove");
    rightMoveId = playerSpr.nameToAnimId("rightMove");
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
        moveDirection = upMoveId;
        if (currentAnimation == upMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_UP) {
        y -= MOVE_SPEED;
        moveDirection = downMoveId;
        if (currentAnimation == downMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_RIGHT) {
        x += MOVE_SPEED;
        moveDirection = rightMoveId;
        if (currentAnimation == rightMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_LEFT) {
        x -= MOVE_SPEED;
        moveDirection = leftMoveId;
        if (currentAnimation == leftMoveId)
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
        if (currentAnimation == upMoveId)
            currentAnimation = upIdleId;
        else if (currentAnimation == downMoveId)
            currentAnimation = downIdleId;
        else if (currentAnimation == leftMoveId)
            currentAnimation = leftIdleId;
        else if (currentAnimation == rightMoveId)
            currentAnimation = rightIdleId;
    } else {
        if (setAnim) {
            currentAnimation = moveDirection;
        }
    }
    Engine::main3dSpr.setSpriteAnim(sprManager, currentAnimation);
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
        ROOMExit* rectExit = room->rectExits[i];
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
        ROOMCollider* collider = &room->roomData.roomColliders.roomColliders[i];
        if (collidesRect(x >> 8, (y >> 8) + 20, 19, 9,
                         collider->x, collider->y,
                         collider->w, collider->h)) {
            if (collider->colliderAction == 0)  // Wall
                return true;
        }
    }
    return false;
}

void Player::draw(Camera &cam) {
    if (sprControl == nullptr)
        return;
    sprControl->x = (x - cam.x) >> 8;
    sprControl->y = (y - cam.y) >> 8;
    sprControl->layer = 100;
}
