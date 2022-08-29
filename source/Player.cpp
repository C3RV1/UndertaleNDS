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
    spriteManager.loadSprite(playerSpr);

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
    char buffer[100];
    int loadSprite = Engine::main3dSpr.loadSprite(spriteManager);
    if (loadSprite != 0) {
        sprintf(buffer, "Error showing player: %d", loadSprite);
        nocashMessage(buffer);
        return;
    }
}

void Player::hidePlayer() {
    Engine::main3dSpr.freeSprite(spriteManager);
}

void Player::update() {
    int32_t prevX = spriteManager.wx, prevY = spriteManager.wy;
    int moveDirection = 0;
    bool setAnim = true;
    if (keysHeld() & KEY_DOWN) {
        spriteManager.wy += MOVE_SPEED;
        moveDirection = upMoveId;
        if (currentAnimation == upMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_UP) {
        spriteManager.wy -= MOVE_SPEED;
        moveDirection = downMoveId;
        if (currentAnimation == downMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_RIGHT) {
        spriteManager.wx += MOVE_SPEED;
        moveDirection = rightMoveId;
        if (currentAnimation == rightMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_LEFT) {
        spriteManager.wx -= MOVE_SPEED;
        moveDirection = leftMoveId;
        if (currentAnimation == leftMoveId)
            setAnim = false;
    }

    if (prevX != spriteManager.wx || prevY != spriteManager.wy) {
        check_exits();
        int x2 = spriteManager.wx, y2 = spriteManager.wy;
        if (check_collisions()) {
            spriteManager.wx = prevX;
            spriteManager.wy = y2;
            if (check_collisions()) {
                spriteManager.wx = x2;
                spriteManager.wy = prevY;
                if (check_collisions()) {
                    spriteManager.wx = prevX;
                    spriteManager.wy = prevY;
                }
            }
        }
    }

    if (spriteManager.wx == prevX && prevY == spriteManager.wy) {
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
    spriteManager.setSpriteAnim(currentAnimation);
}

void Player::check_exits() {
    uint8_t width, height;
    globalRoom->bg.getSize(width, height);
    if (spriteManager.wx < 0) {
        spriteManager.wx = 0;
        if (globalRoom->exitLeft != nullptr) {
            spriteManager.wx = globalRoom->exitLeft->spawnX << 8;
            spriteManager.wy = globalRoom->exitLeft->spawnY << 8;
            loadNewRoom(globalRoom->exitLeft->roomId);
        }
    }
    else if ((spriteManager.wx >> 8) + 19 > width * 8) {
        spriteManager.wx = (width * 8 - 19) << 8;
        if (globalRoom->exitRight != nullptr) {
            spriteManager.wx = globalRoom->exitRight->spawnX << 8;
            spriteManager.wy = globalRoom->exitRight->spawnY << 8;
            loadNewRoom(globalRoom->exitRight->roomId);
        }
    }
    if (spriteManager.wy < 0) {
        spriteManager.wy = 0;
        if (globalRoom->exitTop != nullptr) {
            spriteManager.wx = globalRoom->exitTop->spawnX << 8;
            spriteManager.wy = globalRoom->exitTop->spawnY << 8;
            loadNewRoom(globalRoom->exitTop->roomId);
        }
    }
    else if ((spriteManager.wy >> 8) + 29 > height * 8) {
        spriteManager.wy = (height * 8 - 29) << 8;
        if (globalRoom->exitBtm != nullptr) {
            spriteManager.wx = globalRoom->exitBtm->spawnX << 8;
            spriteManager.wy = globalRoom->exitBtm->spawnY << 8;
            loadNewRoom(globalRoom->exitBtm->roomId);
        }
    }

    for (int i = 0; i < globalRoom->rectExitCount; i++) {
        ROOMExit* rectExit = globalRoom->rectExits[i];
        if (collidesRect(spriteManager.wx >> 8, (spriteManager.wy >> 8) + 20, 19, 9,
                         rectExit->x, rectExit->y,
                         rectExit->w, rectExit->h)) {
            spriteManager.wx = rectExit->spawnX << 8;
            spriteManager.wy = rectExit->spawnY << 8;
            loadNewRoom(rectExit->roomId);
        }
    }
}

bool Player::check_collisions() const {
    for (int i = 0; i < globalRoom->roomData.roomColliders.colliderCount; i++) {
        ROOMCollider* collider = &globalRoom->roomData.roomColliders.roomColliders[i];
        if (collidesRect(spriteManager.wx >> 8, (spriteManager.wy >> 8) + 20, 19, 9,
                         collider->x, collider->y,
                         collider->w, collider->h)) {
            if (collider->colliderAction == 0)  // Wall
                return true;
        }
    }
    return false;
}

void Player::draw() {
    spriteManager.x = spriteManager.wx - globalCamera.x;
    spriteManager.y = spriteManager.wy - globalCamera.y;
    spriteManager.layer = 100;
}

Player* globalPlayer;
