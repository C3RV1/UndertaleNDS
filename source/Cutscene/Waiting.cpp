//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Waiting.hpp"
#include "Battle/Battle.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include "Engine/Engine.hpp"

Waiting::Waiting(Cutscene* cutscene) : _cutscene(cutscene) {}

void Waiting::wait(WaitingType waitingType) {
  if (waitingType == WAIT_FRAMES) {
    Engine::throw_("Wait called with wait frames!");
    return;
  }
  _cWait = waitingType;
}

void Waiting::waitIgnore(WaitingType waitingType) {
  if (_cWait == waitingType)
    _cWait = NONE;
}

void Waiting::waitFrames(int frames) {
  _cWait = WAIT_FRAMES;
  // FIXME: Should this be like this?
  _cWaitTime = frames + 1; // Hack to improve navigation sync
}

void Waiting::update(bool frame) {
  auto &battle = _cutscene->_cBattle;
  switch(_cWait) {
  case NONE:
    break;
  case WAIT_FRAMES:
    if (frame)
      _cWaitTime -= 1;
    if (_cWaitTime <= 0)
      _cWait = NONE;
    break;
  case WAIT_DIALOGUE_END:
    if (_cutscene->_cDialogue == nullptr)
      _cWait = NONE;
    break;
  case WAIT_BATTLE_ATTACK:
    if (battle == nullptr)
      return;
    _cWait = NONE;
    for (size_t i = 0; i < battle->_enemies.size(); i++) {
      if (battle->_cBattleAttacks[i] != nullptr) {
        _cWait = WAIT_BATTLE_ATTACK;
        break;
      }
    }
    break;
  case WAIT_SAVE_MENU:
    if (_cutscene->_cSaveMenu == nullptr)
      _cWait = NONE;
    break;
  case WAIT_BATTLE_ACTION:
    if (battle == nullptr)
      return;
    if (battle->_cBattleAction == nullptr)
      _cWait = NONE;
    break;
  default: // WAIT_EXIT and WAIT_ENTER
    break;
  }
}
