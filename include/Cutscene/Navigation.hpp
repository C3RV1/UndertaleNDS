//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_NAVIGATION_HPP
#define LAYTON_NAVIGATION_HPP

#include "Room.hpp"

class Navigation {
public:
    Navigation(Room** currentRoom);
private:
    Room** currentRoom;
};

#endif //LAYTON_NAVIGATION_HPP
