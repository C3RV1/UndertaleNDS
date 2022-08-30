//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_WAITING_HPP
#define LAYTON_WAITING_HPP

class Waiting;

#include "CutsceneEnums.hpp"

enum WaitingType {
    NONE = 0,
    WAIT_FRAMES,
    WAIT_LOAD,
    WAIT_DIALOGUE_END,
    WAIT_BATTLE_ATTACK,
    WAIT_BATTLE_ACTION
};

class Waiting {
public:
    void waitFrames(int frames);
    void waitLoad();
    void update(CutsceneLocation callingLocation);
    bool getBusy() {return currentWait != NONE;}
private:
    WaitingType currentWait = NONE;
    int currentWaitTime = 0;
};

#endif //LAYTON_WAITING_HPP
