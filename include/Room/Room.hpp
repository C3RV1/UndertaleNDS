//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_ROOM_HPP
#define UNDERTALE_ROOM_HPP

#include <cstdio>
#include <cstring>
#include <vector>
#include "Engine/Engine.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "ManagedSprite.hpp"
#include "Cutscene/Navigation.hpp"

class Room {
public:
    explicit Room(int roomId);
    ~Room() {free_();}
    void loadRoom(FILE *f);
    static bool evaluateCondition(FILE *f);
    void loadSprites();
    void update();
    void draw() const;

    void push();
    void pop();

    u16 _roomId;
    Engine::Background _bg;

    std::vector<std::shared_ptr<Engine::Texture>> _textures;
    std::vector<std::unique_ptr<ManagedSprite>> _sprites;

    ROOMPart _roomData;
    u16 _spawnX = 0, _spawnY = 0;

    ROOMExit* _exitTop = nullptr;
    ROOMExit* _exitBtm = nullptr;
    ROOMExit* _exitLeft = nullptr;
    ROOMExit* _exitRight = nullptr;
    u8 _rectExitCount = 0;
    std::vector<ROOMExit*> _rectExits;
    Navigation _nav;

private:
    void free_();
};

constexpr int kRoomChangeFadeFrames = 20;
void loadNewRoom(int roomId, s32 spawnX, s32 spawnY);

extern std::unique_ptr<Room> globalRoom;

#endif //UNDERTALE_ROOM_HPP
