//
// Created by cervi on 27/08/2022.
//

#ifndef LAYTON_ROOM_FILE_HPP
#define LAYTON_ROOM_FILE_HPP

#include <stdint.h>

struct ROOMHeader {
    char header[4] = {'R', 'O', 'O', 'M'};
    uint32_t fileSize = 0;

    uint32_t version = 3;
};

struct ROOMExit {
    uint8_t exitType = 0; // 0 side, 1 rect
    uint16_t roomId = 0;
    uint16_t spawnX = 0, spawnY = 0;
    union {
        struct {
            uint16_t x = 0, y = 0, w = 0, h = 0;
        };
        struct {
            uint8_t side;  // 0-3, up, down, left, right
        };
    };
};

struct ROOMExits {
    uint8_t exitCount = 0;
    ROOMExit* roomExits = nullptr;
};

struct ROOMSprite {
    char* spritePath = nullptr;
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
    uint16_t cutsceneId = 0;    // only when colliderAction == 1
};

struct ROOMColliders {
    uint16_t colliderCount = 0;
    ROOMCollider* roomColliders = nullptr;
};

struct ROOMPartCondition {
};

struct ROOMPart {
    uint32_t lengthBytes = 0;
    uint8_t conditionCount = 0;
    ROOMPartCondition* conditions = nullptr;
    char* roomBg = nullptr;
    char* musicBg = nullptr;
    ROOMExits roomExits;
    ROOMSprites roomSprites;
    ROOMColliders roomColliders;
};

struct ROOMFile {
    ROOMHeader header;
    uint8_t partCount = 0;
    ROOMPart* parts = nullptr;
};

#endif //LAYTON_ROOM_FILE_HPP
