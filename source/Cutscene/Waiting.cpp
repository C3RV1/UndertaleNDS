//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Waiting.hpp"

void Waiting::waitFrames(int frames) {
    currentWait = WAIT_FRAMES;
    currentWaitTime = frames + 1; // Hack to improve navigation sync
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

void Waiting::waitBattleAttack() {
    currentWait = WAIT_BATTLE_ATTACK;
}

void Waiting::update(CutsceneLocation callingLocation, bool frame) {
    if (currentWait == NONE)
        return;

    if (currentWait == WAIT_FRAMES) {
        if (frame)
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
    } else if (currentWait == WAIT_BATTLE_ATTACK) {
        if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE) {
            if (globalBattle->currentBattleAttack == nullptr) {
                currentWait = NONE;
            }
        } else {
            currentWait = NONE;
        }
    }
}
