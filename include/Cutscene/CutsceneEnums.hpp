//
// Created by cervi on 30/08/2022.
//

#ifndef LAYTON_CUTSCENEENUMS_HPP
#define LAYTON_CUTSCENEENUMS_HPP

enum CutsceneCommands {
    CMD_LOAD_SPRITE = 0,
    CMD_PLAYER_CONTROL = 1,
    CMD_WAIT_EXIT = 2,
    CMD_WAIT_ENTER = 3,
    CMD_SET_SHOWN = 4,
    CMD_SET_ANIMATION = 5,
    CMD_WAIT_FRAMES = 6,
    CMD_SET_POS = 7,
    CMD_MOVE_IN_FRAMES = 8,
    CMD_START_DIALOGUE = 9,
    CMD_WAIT_DIALOGUE_END = 10,
    CMD_START_BATTLE = 11,
    CMD_EXIT_BATTLE = 12,
    CMD_START_BATTLE_DIALOGUE = 13,
    CMD_BATTLE_ATTACK = 14,
    CMD_WAIT_BATTLE_ATTACK = 15,
    CMD_WAIT_BATTLE_ACTION = 16,
    CMD_CMP_BATTLE_ACTION = 17,
    CMD_CHECK_HIT = 18,
    CMD_JUMP_IF = 19,
    CMD_JUMP_IF_NOT = 20,
    CMD_JUMP = 21,
    CMD_MANUAL_CAMERA = 22,
    CMD_UNLOAD_SPRITE = 23,
    CMD_SCALE_IN_FRAMES = 24,
    CMD_SET_SCALE = 25,
    CMD_START_BGM = 26,
    CMD_STOP_BGM = 27,
    CMD_SET_POS_IN_FRAMES = 28,
    CMD_DEBUG = 0xff
};

enum CutsceneLocation {
    LOAD_ROOM = 0,
    ROOM = 1,
    LOAD_BATTLE = 2,
    BATTLE = 3
};

#endif //LAYTON_CUTSCENEENUMS_HPP