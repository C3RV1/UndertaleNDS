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
#include "RoomSprite.hpp"

class Room {
public:
    explicit Room(int roomId);
    int loadRoom(FILE *f);
    bool evaluateCondition(FILE *f);
    void loadSprites();
    void draw(Camera &cam) const;
    void free_();

    uint16_t roomId;
    Engine::Background bg;
    RoomSprite* sprites = nullptr;
    ROOMPart roomData;

    ROOMExit* exitTop = nullptr;
    ROOMExit* exitBtm = nullptr;
    ROOMExit* exitLeft = nullptr;
    ROOMExit* exitRight = nullptr;
    uint8_t rectExitCount = 0;
    ROOMExit** rectExits = nullptr;
};

const int ROOM_CHANGE_FADE_FRAMES = 20;
void loadNewRoom(Room*& room, Camera& cam, Player& player, int roomId);

#endif //LAYTON_ROOM_HPP
