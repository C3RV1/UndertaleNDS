//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_WAITING_HPP
#define UNDERTALE_WAITING_HPP

#include "CutsceneEnums.hpp"

class Waiting {
public:
    void wait(WaitingType waitingType);
    void waitFrames(int frames);
    void update(CutsceneLocation callingLocation, bool frame);
    bool getBusy() {return currentWait != NONE;}
private:
    WaitingType currentWait = NONE;
    int currentWaitTime = 0;
};

#endif //UNDERTALE_WAITING_HPP
