//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Waiting.hpp"
#include "Battle/Battle.hpp"
#include "Cutscene/Cutscene.hpp"

void Waiting::wait(WaitingType waitingType) {
    if (waitingType == WAIT_FRAMES) {
        nocashMessage("Wait called with wait frames?");
        return;
    }
    currentWait = waitingType;
}

void Waiting::waitFrames(int frames) {
    currentWait = WAIT_FRAMES;
    currentWaitTime = frames + 1; // Hack to improve navigation sync
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
        if (globalCutscene == nullptr)
            currentWait = NONE;
        else if (globalCutscene->cDialogue == nullptr)
            currentWait = NONE;
    } else if (currentWait == WAIT_BATTLE_ATTACK) {
        if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE) {
            if (globalBattle->currentBattleAttack == nullptr) {
                currentWait = NONE;
            }
        } else {
            currentWait = NONE;
        }
    } else if (currentWait == WAIT_SAVE_MENU) {
        if (globalCutscene == nullptr)
            currentWait = NONE;
        else if (globalCutscene->cSaveMenu == nullptr)
            currentWait = NONE;
    } else if (currentWait == WAIT_BATTLE_ACTION) {
        if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE) {
            if (globalBattle->currentBattleAction == nullptr) {
                currentWait = NONE;
            }
        } else {
            currentWait = NONE;
        }
    }
}
