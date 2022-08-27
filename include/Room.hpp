//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_ROOM_HPP
#define LAYTON_ROOM_HPP

class Room;

#include <stdio.h>
#include <string.h>
#include "Engine/Background.hpp"
#include "Engine/Engine.hpp"
#include "Camera.hpp"
#include "Player.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Formats/utils.hpp"

class Room {
public:
    explicit Room(int roomId);
    int loadRoom(FILE *f);
    bool evaluateCondition(FILE *f);
    // void loadSprites();
    // void update();
    // void draw(Camera &cam);
    void free_();
    uint16_t roomId;
    Engine::Background bg;
    Engine::Sprite* sprites = nullptr;
    Engine::SpriteManager* spriteManagers;
    Engine::SpriteControl* spriteControls;
    RoomPart roomData;

    RoomExit* exitTop = nullptr;
    RoomExit* exitBtm = nullptr;
    RoomExit* exitLeft = nullptr;
    RoomExit* exitRight = nullptr;
    uint8_t rectExitCount = 0;
    RoomExit** rectExits = nullptr;
};

const int ROOM_CHANGE_FADE_FRAMES = 80;
void loadNewRoom(Room*& room, Camera& cam, Player& player, int roomId);

#endif //LAYTON_ROOM_HPP
