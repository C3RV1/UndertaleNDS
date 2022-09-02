//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Waiting.hpp"

void Waiting::waitFrames(int frames) {
    currentWait = WAIT_FRAMES;
    currentWaitTime = frames;
}

void Waiting::waitExit() {
    currentWait = WAIT_EXIT;
}

void Waiting::waitEnter() {
    currentWait = WAIT_ENTER;
}

void Waiting::waitDialogueEnd() {
    currentWait = WAIT_DIALOGUE_END;
}

void Waiting::update(CutsceneLocation callingLocation) {
    if (currentWait == NONE)
        return;

    if (currentWait == WAIT_FRAMES) {
        currentWaitTime -= 1;
        if (currentWaitTime <= 0) {
            currentWait = NONE;
        }
    } else if (currentWait == WAIT_EXIT) {
        if (callingLocation == LOAD_ROOM || callingLocation == LOAD_BATTLE)
            currentWait = NONE;
    } else if (currentWait == WAIT_ENTER) {
        if (callingLocation == ROOM || callingLocation == BATTLE)
            currentWait = NONE;
    } else if (currentWait == WAIT_DIALOGUE_END) {
        if (currentDialogue == nullptr)
            currentWait = NONE;
    }
}
