//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_WAITING_HPP
#define UNDERTALE_WAITING_HPP

#include "CutsceneEnums.hpp"
#include "nds.h"

class Cutscene;

class Waiting {
public:
    explicit Waiting(Cutscene* cutscene);
    void wait(WaitingType waitingType);
    void waitFrames(int frames);
    void waitIgnore(WaitingType waitingType);
    void update(bool frame);
    bool getBusy() {return _cWait != 0;}
private:
    Cutscene* _cutscene;
    WaitingType _cWait = NONE;
    int _cWaitTime = 0;
};

#endif //UNDERTALE_WAITING_HPP
