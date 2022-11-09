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
    bool getBusy() {return _cWait != NONE;}
private:
    WaitingType _cWait = NONE;
    int _cWaitTime = 0;
};

#endif //UNDERTALE_WAITING_HPP
