//
// Created by cervi on 27/08/2022.
//

#ifndef LAYTON_ROOM_FILE_HPP
#define LAYTON_ROOM_FILE_HPP

#include <stdint.h>

struct ROOMHeader {
    char header[4] = {'R', 'O', 'O', 'M'};
    uint32_t fileSize = 0;

    uint32_t version = 4;
};

struct ROOMExit {
    uint8_t exitType = 0; // 0 side, 1 rect
    uint16_t roomId = 0;
    uint16_t spawnX = 0, spawnY = 0;
    uint16_t x = 0, y = 0, w = 0, h = 0;  // rect
    uint8_t side = 0;  // side 0-3, up, down, left, right
};

struct ROOMExits {
    uint8_t exitCount = 0;
    ROOMExit* roomExits = nullptr;
};

struct ROOMTextures {
    uint8_t textureCount = 0;
    char** texturePaths = nullptr;
};

struct ROOMSprite {
    int8_t textureId = 0;
    uint16_t x = 0, y = 0, layer = 0;
    char* animation = nullptr;
    bool canInteract = false;
    uint8_t interactAction = 0;  // 0 - save, 1 - cutscene
    uint16_t cutsceneId = 0;  // only when interactAction == 1
};

struct ROOMSprites {
    uint8_t spriteCount = 0;
    ROOMSprite* roomSprites = nullptr;
};

struct ROOMCollider {
    uint16_t x = 0, y = 0, w = 0, h = 0;
    uint8_t colliderAction = 0;  // 0 - wall, 1 - trigger
    bool enabled = true;
    uint16_t cutsceneId = 0;    // only when colliderAction == 1
};

struct ROOMColliders {
    uint16_t colliderCount = 0;
    ROOMCollider* roomColliders = nullptr;
};

struct ROOMPartCondition {
    uint16_t flagId = 0;
    uint8_t cmpOperator = 0;  // Same as cutscene cmp, but bit 4 is flip
    uint16_t cmpValue = 0;
};

struct ROOMPart {
    uint32_t lengthBytes = 0;
    uint8_t conditionCount = 0;
    ROOMPartCondition* conditions = nullptr;
    char roomBg[50] = {0};
    char musicBg[50] = {0};
    ROOMExits roomExits;
    ROOMTextures roomTextures;
    ROOMSprites roomSprites;
    ROOMColliders roomColliders;
};

struct ROOMFile {
    ROOMHeader header;
    uint8_t partCount = 0;
    ROOMPart* parts = nullptr;
};

#endif //LAYTON_ROOM_FILE_HPP
