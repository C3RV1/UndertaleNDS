//
// Created by cervi on 24/08/2022.
//

#ifndef LAYTON_SAVE_HPP
#define LAYTON_SAVE_HPP

struct SaveData {
    char* name = nullptr;
    int currentRoom = 0;
};

extern SaveData saveGlobal;

#endif //LAYTON_SAVE_HPP
