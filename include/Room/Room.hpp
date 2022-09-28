//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_ROOM_HPP
#define UNDERTALE_ROOM_HPP

#include <stdio.h>
#include <string.h>
#include "Engine/Engine.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "ManagedSprite.hpp"
#include "Cutscene/Navigation.hpp"

class Room {
public:
    explicit Room(int roomId);
    int loadRoom(FILE *f);
    static bool evaluateCondition(FILE *f);
    void loadSprites();
    void update();
    void draw() const;
    void free_();

    void push();
    void pop();

    u16 roomId;
    Engine::Background bg;

    u8 textureCount = 0;
    Engine::Texture** textures = nullptr;

    u8 spriteCount = 0;
    ManagedSprite** sprites = nullptr;

    ROOMPart roomData;
    u16 spawnX = 0, spawnY = 0;

    ROOMExit* exitTop = nullptr;
    ROOMExit* exitBtm = nullptr;
    ROOMExit* exitLeft = nullptr;
    ROOMExit* exitRight = nullptr;
    u8 rectExitCount = 0;
    ROOMExit** rectExits = nullptr;
    Navigation nav;
};

const int ROOM_CHANGE_FADE_FRAMES = 20;
void loadNewRoom(int roomId, s32 spawnX, s32 spawnY);

extern Room* globalRoom;

#endif //UNDERTALE_ROOM_HPP
