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
    _cWait = waitingType;
}

void Waiting::waitFrames(int frames) {
    _cWait = WAIT_FRAMES;
    _cWaitTime = frames + 1; // Hack to improve navigation sync
}

void Waiting::update(CutsceneLocation callingLocation, bool frame) {
    if (_cWait == NONE)
        return;

    if (_cWait == WAIT_FRAMES) {
        if (frame)
            _cWaitTime -= 1;
        if (_cWaitTime <= 0) {
            _cWait = NONE;
        }
    } else if (_cWait == WAIT_EXIT) {
        if (callingLocation == LOAD_ROOM || callingLocation == LOAD_BATTLE)
            _cWait = NONE;
    } else if (_cWait == WAIT_ENTER) {
        if (callingLocation == ROOM || callingLocation == BATTLE)
            _cWait = NONE;
    } else if (_cWait == WAIT_DIALOGUE_END) {
        if (globalCutscene == nullptr)
            _cWait = NONE;
        else if (globalCutscene->_cDialogue == nullptr)
            _cWait = NONE;
    } else if (_cWait == WAIT_BATTLE_ATTACK) {
        if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE) {
            _cWait = NONE;
            for (int i = 0; i < globalBattle->_enemies.size(); i++) {
                if (globalBattle->_cBattleAttacks[i] != nullptr) {
                    _cWait = WAIT_BATTLE_ATTACK;
                    break;
                }
            }
        } else {
            _cWait = NONE;
        }
    } else if (_cWait == WAIT_SAVE_MENU) {
        if (globalCutscene == nullptr)
            _cWait = NONE;
        else if (globalCutscene->_cSaveMenu == nullptr)
            _cWait = NONE;
    } else if (_cWait == WAIT_BATTLE_ACTION) {
        if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE) {
            if (globalBattle->_cBattleAction == nullptr) {
                _cWait = NONE;
            }
        } else {
            _cWait = NONE;
        }
    }
}
