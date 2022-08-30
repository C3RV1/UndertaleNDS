//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_CUTSCENE_HPP
#define LAYTON_CUTSCENE_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Navigation.hpp"

enum CutsceneCommands {
    LOAD_SPRITE = 0,
    PLAYER_CONTROL = 1,
    WAIT_LOAD = 2,
    SHOW = 3,
    HIDE = 4,
    SET_ANIMATION = 5,
    WAIT_FRAMES = 6,
    SET_X_Y = 7,
    MOVE_IN_FRAMES = 8,
    START_DIALOGUE = 9,
    WAIT_DIALOGUE_END = 10,
    START_BATTLE = 11,
    EXIT_BATTLE = 12,
    START_BATTLE_DIALOGUE = 13,
    BATTLE_ATTACK = 14,
    WAIT_BATTLE_ATTACK = 15,
    WAIT_BATTLE_ACTION = 16,
    CMP_BATTLE_ACTION = 17,
    CHECK_HIT = 18,
    JUMP_IF = 19,
    JUMP_IF_NOT = 20,
    JUMP = 21,
    MANUAL_CAMERA = 22,
    UNLOAD_SPRITE = 23,
    DEBUG = 0xff
};

class Cutscene {
public:
    Cutscene(uint16_t cutsceneId, Navigation* navigation);
private:
    FILE* commandStream;
};

#endif //LAYTON_CUTSCENE_HPP
