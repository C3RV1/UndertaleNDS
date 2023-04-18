//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_ROOM_HPP
#define UNDERTALE_ROOM_HPP

#include <cstdio>
#include <cstring>
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

    u16 _roomId;
    Engine::Background _bg;

    u8 _textureCount = 0;
    Engine::Texture** _textures = nullptr;

    u8 _spriteCount = 0;
    ManagedSprite** _sprites = nullptr;

    ROOMPart _roomData;
    u16 _spawnX = 0, _spawnY = 0;

    ROOMExit* _exitTop = nullptr;
    ROOMExit* _exitBtm = nullptr;
    ROOMExit* _exitLeft = nullptr;
    ROOMExit* _exitRight = nullptr;
    u8 _rectExitCount = 0;
    ROOMExit** _rectExits = nullptr;
    Navigation _nav;
};

constexpr int kRoomChangeFadeFrames = 20;
void loadNewRoom(int roomId, s32 spawnX, s32 spawnY);

extern Room* globalRoom;

#endif //UNDERTALE_ROOM_HPP
