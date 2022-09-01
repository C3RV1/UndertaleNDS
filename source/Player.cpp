//
// Created by cervi on 27/08/2022.
//

#include "Player.hpp"

Player::Player() : spriteManager(Engine::Allocated3D) {
    char buffer[100];
    FILE *f = fopen("nitro:/spr/mainchara.cspr", "rb");
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

    upIdleId = spriteManager.nameToAnimId("upIdle");
    downIdleId = spriteManager.nameToAnimId("downIdle");
    leftIdleId = spriteManager.nameToAnimId("leftIdle");
    rightIdleId = spriteManager.nameToAnimId("rightIdle");
    upMoveId = spriteManager.nameToAnimId("upMove");
    downMoveId = spriteManager.nameToAnimId("downMove");
    leftMoveId = spriteManager.nameToAnimId("leftMove");
    rightMoveId = spriteManager.nameToAnimId("rightMove");
}

void Player::update() {
    if (!playerControl)
        return;
    int32_t prevX = spriteManager.wx, prevY = spriteManager.wy;
    int moveDirection = 0;
    bool setAnim = true;
    if (keysHeld() & KEY_DOWN) {
        spriteManager.wy += MOVE_SPEED;
        moveDirection = downMoveId;
        if (currentAnimation == downMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_UP) {
        spriteManager.wy -= MOVE_SPEED;
        moveDirection = upMoveId;
        if (currentAnimation == upMoveId)
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
    uint16_t width, height;
    globalRoom->bg.getSize(width, height);
    if (spriteManager.wx < 0) {
        spriteManager.wx = 0;
        if (globalRoom->exitLeft != nullptr) {
            loadNewRoom(globalRoom->exitLeft->roomId,
                        globalRoom->exitLeft->spawnX,
                        globalRoom->exitLeft->spawnY);
        }
    }
    else if ((spriteManager.wx >> 8) + 19 > width * 8) {
        spriteManager.wx = (width * 8 - 19) << 8;
        if (globalRoom->exitRight != nullptr) {
            loadNewRoom(globalRoom->exitRight->roomId,
                        globalRoom->exitRight->spawnX,
                        globalRoom->exitLeft->spawnY);
        }
    }
    if (spriteManager.wy < 0) {
        spriteManager.wy = 0;
        if (globalRoom->exitTop != nullptr) {
            loadNewRoom(globalRoom->exitTop->roomId,
                        globalRoom->exitTop->spawnX,
                        globalRoom->exitTop->spawnY);
        }
    }
    else if ((spriteManager.wy >> 8) + 29 > height * 8) {
        spriteManager.wy = (height * 8 - 29) << 8;
        if (globalRoom->exitBtm != nullptr) {
            loadNewRoom(globalRoom->exitBtm->roomId,
                        globalRoom->exitBtm->spawnX,
                        globalRoom->exitBtm->spawnY);
        }
    }

    for (int i = 0; i < globalRoom->rectExitCount; i++) {
        ROOMExit* rectExit = globalRoom->rectExits[i];
        if (collidesRect(spriteManager.wx >> 8, (spriteManager.wy >> 8) + 20, 19, 9,
                         rectExit->x, rectExit->y,
                         rectExit->w, rectExit->h)) {
            loadNewRoom(rectExit->roomId,
                        rectExit->spawnX,
                        rectExit->spawnY);
        }
    }
}

bool Player::check_collisions() const {
    for (int i = 0; i < globalRoom->roomData.roomColliders.colliderCount; i++) {
        ROOMCollider* collider = &globalRoom->roomData.roomColliders.roomColliders[i];
        if (!collider->enabled)
            continue;
        if (collidesRect(spriteManager.wx >> 8, (spriteManager.wy >> 8) + 20, 19, 9,
                         collider->x, collider->y,
                         collider->w, collider->h)) {
            if (collider->colliderAction == 0)  // Wall
                return true;
            if (collider->colliderAction == 1 && globalCutscene == nullptr) {  // Trigger
                globalCutscene = new Cutscene(collider->cutsceneId);
            }
        }
    }
    return false;
}

void Player::draw() {
    spriteManager.cam_x = globalCamera.pos.wx;
    spriteManager.cam_y = globalCamera.pos.wy;
    spriteManager.cam_scale_x = globalCamera.pos.wscale_x;
    spriteManager.cam_scale_y = globalCamera.pos.wscale_y;
    spriteManager.layer = 100;
}

Player* globalPlayer;
