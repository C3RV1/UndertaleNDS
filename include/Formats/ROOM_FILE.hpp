//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_ROOM_FILE_HPP
#define UNDERTALE_ROOM_FILE_HPP

#include <nds.h>

struct ROOMHeader {
    char header[4] = {'R', 'O', 'O', 'M'};
    u32 fileSize = 0;

    u32 version = 9;
    static constexpr u32 version_expected = 9;
};

struct ROOMExit {
    u8 exitType = 0; // 0 side, 1 rect
    u16 roomId = 0;
    u16 spawnX = 0, spawnY = 0;
    u16 x = 0, y = 0, w = 0, h = 0;  // rect
    u8 side = 0;  // side 0-3, up, down, left, right
};

struct ROOMExits {
    std::vector<ROOMExit> roomExits;
};

struct ROOMTextures {
    std::vector<std::string> texturePaths;
};

struct ROOMSprite {
    s8 textureId = 0;
    u16 x = 0, y = 0;
    std::string animation;
    u8 interactAction = 0;  // 0 - none, 1 - cutscene, 2 - proximity, 3 - parallax

    u16 cutsceneId = 0;  // only when interactAction == 1

    // only when interactAction == 2
    u16 distance = 0;
    std::string closeAnim;

    // only when interactAction == 3
    s32 parallax_x, parallax_y;
};

struct ROOMSprites {
    std::vector<ROOMSprite> roomSprites;
};

struct ROOMCollider {
    u16 x = 0, y = 0, w = 0, h = 0;
    u8 colliderAction = 0;  // 0 - wall, 1 - cutscene
    bool enabled = true;
    u16 cutsceneId = 0;    // only when colliderAction == 1
};

struct ROOMColliders {
    std::vector<ROOMCollider> roomColliders;
};

struct ROOMPartCondition {
    u16 flagId = 0;
    u8 cmpOperator = 0;  // Same as cutscene cmp, but bit 4 is flip
    u16 cmpValue = 0;
};

struct ROOMPart {
    u32 lengthBytes = 0;
    u8 conditionCount = 0;
    ROOMPartCondition* conditions = nullptr;
    std::string roomBg;
    std::string musicBg;
    u8 musicVolume;
    u16 spawnX = 0, spawnY = 0;
    ROOMExits roomExits;
    ROOMTextures roomTextures;
    ROOMSprites roomSprites;
    ROOMColliders roomColliders;
};

struct ROOMFile {
    ROOMHeader header;
    u8 partCount = 0;
    ROOMPart* parts = nullptr;
};

#endif //UNDERTALE_ROOM_FILE_HPP
