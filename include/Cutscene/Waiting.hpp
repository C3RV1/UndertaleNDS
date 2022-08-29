//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_WAITING_HPP
#define LAYTON_WAITING_HPP

enum WaitingType {
    NONE = 0,
    FRAMES,
    LOAD,
    DIALOGUE_END,
    BATTLE_ATTACK,
    BATTLE_ACTION
};

class Waiting {
public:
    void WaitFrames();
private:
    WaitingType currentWait = NONE;
    int currentWaitTime = 0;
};

#endif //LAYTON_WAITING_HPP
