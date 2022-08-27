//
// Created by cervi on 27/08/2022.
//

#ifndef LAYTON_ROOM_FILE_HPP
#define LAYTON_ROOM_FILE_HPP

#include <stdint.h>

struct RoomHeader {
    char header[4] = {'R', 'O', 'O', 'M'};
    uint32_t fileSize = 0;

    uint32_t version = 1;
};

struct RoomExit {
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

struct RoomExits {
    uint8_t exitCount = 0;
    RoomExit* roomExits = nullptr;
};

struct RoomSprite {
    char* spritePath = nullptr;
    uint8_t animationStart = 0;
    uint8_t animationLength = 0;
    uint8_t frameTime = 0;
    bool canInteract = false;
    uint8_t interactAction = 0;  // 0 - save, 1 - cutscene
    uint16_t cutsceneId = 0;  // only when interactAction == 1
};

struct RoomSprites {
    uint8_t spriteCount = 0;
    RoomSprite* roomSprites = nullptr;
};

struct RoomCollider {
    uint16_t x, y, w, h;
};

struct RoomColliders {
    uint16_t colliderCount = 0;
    RoomCollider* roomColliders = nullptr;
};

struct RoomPartCondition {
};

struct RoomPart {
    uint32_t lengthBytes = 0;
    uint8_t conditionCount = 0;
    RoomPartCondition* conditions = nullptr;
    uint16_t onEnterCinematic = 0;
    char* roomBg = nullptr;
    char* musicBg = nullptr;
    RoomExits roomExits;
    RoomSprites roomSprites;
    RoomColliders roomColliders;
};

struct RoomFile {
    RoomHeader header;
    uint8_t partCount = 0;
    RoomPart* parts = nullptr;
};

#endif //LAYTON_ROOM_FILE_HPP
