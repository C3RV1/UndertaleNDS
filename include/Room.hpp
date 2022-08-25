//
// Created by cervi on 25/08/2022.
//

#ifndef LAYTON_ROOM_HPP
#define LAYTON_ROOM_HPP

#include <stdio.h>

class Room {
public:
    virtual void load();
    virtual void loop();
private:
    uint16_t width = 0, height = 0;
};

extern Room* currentRoom;

#endif //LAYTON_ROOM_HPP
