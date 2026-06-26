//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"
#include "Battle/Battle.hpp"
#include "Battle/FlavorTextDialogue.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Cutscene/Navigation.hpp"
#include "Engine/Audio.hpp"
#include "Fader.hpp"
#include "Formats/CSCN.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Formats/utils.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include <memory>
#include <string>

Cutscene::Cutscene(u16 cutsceneId, u16 roomId, Room* room)
    : _cutsceneId(cutsceneId), _roomId(roomId), _room(room), _waiting(this) {
  std::string buffer = "nitro:/cutscenes/r" + std::to_string(roomId) + "/c" +
                       std::to_string(cutsceneId) + ".cscn";
  FILE *f = fopen(buffer.c_str(), "rb");
  if (f) {
    setvbuf(f, NULL, _IOFBF, 4 * 1024);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    _commandData.openFromFile(f, len);

    if (!checkHeader()) {
      buffer = "Error cutscene " + std::to_string(cutsceneId) + ": HEADER";
      nocashMessage(buffer.c_str());
      _commandData.close();
    }
  } else {
    buffer = "Error opening cutscene " + std::to_string(cutsceneId);
    nocashMessage(buffer.c_str());
  }
  fclose(f);
}

bool Cutscene::checkHeader() {
  char header[4];
  char expectedHeader[4] = {'C', 'S', 'C', 'N'};

  _commandData.read(&header, 4);
  if (memcmp(header, expectedHeader, 4) != 0)
    return false;

  u32 version;
  _commandData.read(&version, 4);
  if (version != CSCN::version)
    return false;

  u32 fileSize;
  _commandData.read(&fileSize, 4);
  if (_commandData.size() != fileSize)
    return false;

  return true;
}

void Cutscene::update() {
  if (!_fader.fadeFinished()) {
    _fader.update();
    if (!_fader.fadeFinished())
      return;
    switch(_fader.getLastFadeType()) {
    case FadeType::FADE_OUT:
      _waiting.waitIgnore(WaitingType::WAIT_EXIT);
      if (_cBattle && _cBattle->_running) {
        _room->push();
        lcdMainOnBottom();
        _cBattle->enter();
      }
      else {
        _cBattle = nullptr;
        lcdMainOnTop();
        _room->pop();
      }
      _fader.startFade(FadeType::FADE_IN);
      break;
    case FadeType::FADE_IN:
      _waiting.waitIgnore(WaitingType::WAIT_ENTER);
      break;
    default:
      break;
    }
  }
  
  if (_cBattle) {
    _cBattle->update();
    if (!_cBattle->_running) {
      _fader.startFade(FadeType::FADE_OUT);
    }
  }

  if (_cDialogue) {
    if (_cDialogue->update()) {
      _cDialogue = nullptr;
    }
  } else if (_cSaveMenu) {
    if (_cSaveMenu->update()) {
      _cSaveMenu = nullptr;
    }
  }
}

bool Cutscene::runCommands() {
  _waiting.update(true);
  if (_waiting.getBusy())
    return false;
  if (_commandData.at_end() && _fader.fadeFinished())
    return true;

  while (!_waiting.getBusy() && !_commandData.at_end()) {
    u8 cmd;
    _commandData.read(&cmd, 1);
    
    runCommand(cmd);
    
    _waiting.update(false);
  }

  return false;
}

bool Cutscene::runCommand(u8 cmd) {
  /*int len;
  TargetInfo targetInfo;
  u32 address;*/
  u32 address;
  
  Navigation* nav;
  if (_cBattle)
    nav = &_cBattle->_nav;
  else
    nav = &_room->_nav;

  switch (cmd) {
  case CMD_DEBUG: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_DEBUG");
#endif
    std::string buffer = _commandData.readstring();
    nocashMessage(buffer.c_str());
    break;
  }
  case CMD_LOAD_SPRITE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_LOAD_SPRITE");
#endif
    s32 x, y, layer;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    _commandData.read(&layer, 4);
    std::string path = _commandData.readstring();
    nav->spawn_sprite(path, x, y, layer);
    break;
  }
  case CMD_UNLOAD_SPRITE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_UNLOAD_SPRITE");
#endif
    s8 sprId;
    _commandData.read(&sprId, 1);
    nav->unload_sprite(sprId);
    break;
  }
  case CMD_PLAYER_CONTROL: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_PLAYER_CONTROL");
#endif
    bool playerControl;
    _commandData.read(&playerControl, 1);
    _room->_player.set_player_control(playerControl);
    if (playerControl)
      _room->_ingame_menu->show();
    else
      _room->_ingame_menu->hide();
    break;
  }
  case CMD_MANUAL_CAMERA: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_MANUAL_CAMERA");
#endif
    bool manualCamera;
    _commandData.read(&manualCamera, 1);
    _room->_camera._manual = manualCamera;
    break;
  }
  case CMD_WAIT: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_WAIT");
#endif
    u8 waitType;
    _commandData.read(&waitType, 1);
    if (waitType == WAIT_FRAMES) {
      u16 frames;
      _commandData.read(&frames, 2);
      _waiting.waitFrames(frames);
      break;
    }
    _waiting.wait((WaitingType)(waitType));
    break;
  }
  case CMD_SET_SHOWN: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_SHOWN");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    bool shown;
    _commandData.read(&shown, 1);
    nav->set_shown(targetInfo, shown);
    break;
  }
  case CMD_SET_ANIMATION: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_ANIMATION");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    std::string anim = _commandData.readstring();
    nav->set_animation(targetInfo, anim);
    break;
  }
  case CMD_SET_OPACITY: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_OPACITY");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    u8 opacity;
    _commandData.read(&opacity, 1);
    nav->set_opacity(targetInfo, opacity);
    break;
  }
  case CMD_SET_POS: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_POS");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    nav->set_position(targetInfo, x, y);
    break;
  }
  case CMD_MOVE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_MOVE");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 dx, dy;
    _commandData.read(&dx, 4);
    _commandData.read(&dy, 4);
    nav->move(targetInfo, dx, dy);
    break;
  }
  case CMD_SET_SCALE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_SCALE");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    nav->set_scale(targetInfo, x, y);
    break;
  }
  case CMD_SET_POS_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_POS_IN_FRAMES");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    u16 frames;
    _commandData.read(&frames, 2);
    nav->set_pos_in_frames(targetInfo, x, y, frames);
    break;
  }
  case CMD_MOVE_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_MOVE_IN_FRAMES");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    u16 frames;
    _commandData.read(&frames, 2);
    nav->move_in_frames(targetInfo, x, y, frames);
    break;
  }
  case CMD_SCALE_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SCALE_IN_FRAMES");
#endif
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    u16 frames;
    _commandData.read(&frames, 2);
    nav->scale_in_frames(targetInfo, x, y, frames);
    break;
  }
  case CMD_START_DIALOGUE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_START_DIALOGUE");
#endif
    TargetInfo targetInfo;
    u16 textId, framesPerLetter;
    s32 x, y;
    std::string speaker, font, speakerIdle, speakerTalk, targetIdle, targetTalk,
        typeSnd;
    bool mainScreen;
    DialogueType dialogue_type;

    _commandData.read(&dialogue_type, 1);
    _commandData.read(&textId, 2);

    if (dialogue_type == DIALOGUE_CENTERED)
      speaker = _commandData.readstring();

    if (dialogue_type != DIALOGUE_FLAVOR_TEXT) {
      _commandData.read(&x, 4);
      _commandData.read(&y, 4);
    }

    if (dialogue_type == DIALOGUE_CENTERED) {
      speakerIdle = _commandData.readstring();
      speakerTalk = _commandData.readstring();
    }

    if (dialogue_type != DIALOGUE_FLAVOR_TEXT) {
      targetInfo = readTarget(_commandData);
      targetIdle = _commandData.readstring();
      targetTalk = _commandData.readstring();
    }

    typeSnd = _commandData.readstring();
    font = _commandData.readstring();

    _commandData.read(&framesPerLetter, 2);

    if (dialogue_type != DIALOGUE_FLAVOR_TEXT)
      _commandData.read(&mainScreen, 1);

    Engine::TextBGManager &txt =
        mainScreen ? Engine::textMain : Engine::textSub;
    Engine::AllocationMode heartAlloc =
        mainScreen ? Engine::Allocated3D : Engine::AllocatedOAM;

    auto target = nav->getTarget(targetInfo);
    if (_cDialogue == nullptr) {
      if (dialogue_type == DIALOGUE_CENTERED)
        _cDialogue = std::make_unique<DialogueCentered>(
            _room->_save.get(), _cutsceneId, _room->_roomId, textId, speaker, x,
            y, speakerIdle, speakerTalk, target, targetIdle, targetTalk,
            typeSnd, font, framesPerLetter, txt, heartAlloc);
      else if (dialogue_type == DIALOGUE_LEFT_ALIGNED)
        _cDialogue = std::make_unique<DialogueLeftAligned>(
            _room->_save.get(), _cutsceneId, _room->_roomId, textId, x, y,
            target, targetIdle, targetTalk, typeSnd, font, framesPerLetter, txt,
            heartAlloc);
      else if (dialogue_type == DIALOGUE_FLAVOR_TEXT && _cBattle)
        _cDialogue = std::make_unique<FlavorTextDialogue>(
            _cBattle.get(), _cutsceneId, _room->_roomId, textId, typeSnd, font,
            framesPerLetter);
    }
    break;
  }
  case CMD_START_BATTLE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_START_BATTLE");
#endif
    if (_cBattle == nullptr) {
      _cBattle = std::make_unique<Battle>(this);
      _cBattle->loadFromBuffer(_commandData);
      _fader.startFade(FadeType::FADE_OUT);
    }
    return true;
  }
  case CMD_EXIT_BATTLE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_EXIT_BATTLE");
#endif
    bool battleWon = false;
    _commandData.read(&battleWon, 1);
    if (_cBattle != nullptr)
      _cBattle->exit(battleWon);
    return true;
  }
  case CMD_BATTLE_ATTACK: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_BATTLE_ATTACK");
#endif
    if (_cBattle) // just in case
      _cBattle->startBattleAttacks();
      
    break;
  }
  case CMD_BATTLE_ACTION: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_BATTLE_ACTION");
#endif
    if (_cBattle == nullptr) // just in case
      break;
    if (_cBattle->_cBattleAction != nullptr)
      break;
    _cBattle->hide();

    s16 flavorTextId;
    _commandData.read(&flavorTextId, 2);

    _cBattle->_cBattleAction =
        std::make_unique<BattleAction>(_cBattle.get(), &_cBattle->_enemies, flavorTextId);
    break;
  }
  case CMD_JUMP_IF:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_JUMP_IF");
#endif
    _commandData.read(&address, 4);
    if (_flag)
      _commandData.seek(address);
    break;
  case CMD_JUMP_IF_NOT:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_JUMP_IF_NOT");
#endif
    _commandData.read(&address, 4);
    if (!_flag)
      _commandData.seek(address);
    break;
  case CMD_JUMP:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_JUMP");
#endif
    _commandData.read(&address, 4);
    _commandData.seek(address);
    break;
  case CMD_START_BGM: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_START_BGM");
#endif
    bool loop;
    _commandData.read(&loop, 1);
    std::string path = _commandData.readstring();
    Audio2::playBGMusic(path, loop);
    break;
  }
  case CMD_STOP_BGM:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_STOP_BGM");
#endif
    Audio2::stopBGMusic();
    break;
  case CMD_PLAY_SFX: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_PLAY_SFX");
#endif
    s8 loops;
    _commandData.read(&loops, 1);
    std::string path = _commandData.readstring();

    auto sfxWav = std::make_shared<Audio2::WAV>();
    sfxWav->load(path);
    sfxWav->setLoops(loops);
    Audio2::audioManager.play(std::move(sfxWav));
    break;
  }
  case CMD_SET_FLAG: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_FLAG");
#endif
    u16 flagId, flagValue;
    _commandData.read(&flagId, 2);
    _commandData.read(&flagValue, 2);
    _room->_save->flags[flagId] = flagValue;
    _room->_save->writePermanentFlags();
    break;
  }
  case CMD_MOD_FLAG: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_MOD_FLAG");
#endif
    u16 flagId;
    s16 flagMod;
    _commandData.read(&flagId, 2);
    _commandData.read(&flagMod, 2);
    _room->_save->flags[flagId] += flagMod;
    _room->_save->writePermanentFlags();
    break;
  }
  case CMD_CMP_FLAG: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_CMP_FLAG");
#endif
    u16 flagId, flagValue, cmpValue;
    u8 comparator;
    _commandData.read(&flagId, 2);
    _commandData.read(&comparator, 1);
    _commandData.read(&cmpValue, 2);
    flagValue = _room->_save->flags[flagId];
    if ((comparator & 3) == ComparisonOperator::EQUALS)
      _flag = (flagValue == cmpValue);
    else if ((comparator & 3) == ComparisonOperator::GREATER_THAN)
      _flag = (flagValue > cmpValue);
    else if ((comparator & 3) == ComparisonOperator::LESS_THAN)
      _flag = (flagValue < cmpValue);
    if (comparator & 4)
      _flag = !_flag;
    break;
  }
  case CMD_SET_COLLIDER_ENABLED: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_COLLIDER_ENABLED");
#endif
    u8 colliderId;
    bool enabled;
    _commandData.read(&colliderId, 1);
    _commandData.read(&enabled, 1);
    if (colliderId < _room->_roomData.roomColliders.roomColliders.size()) {
      _room->_roomData.roomColliders.roomColliders[colliderId].enabled =
          enabled;
    }
    break;
  }
  case CMD_SET_ACTION: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_ACTION");
#endif
    // TODO: IMPROVE
    u8 interactAction;
    u16 cutsceneId_;
    TargetInfo targetInfo = readTarget(_commandData);
    _commandData.read(&interactAction, 1);
    
    if (interactAction == 1)
      _commandData.read(&cutsceneId_, 2);
    
    u8 targetId2 = 0;

    if (targetInfo.targetId < 0)
      targetId2 = _room->_sprites.size() + targetInfo.targetId;
    else
      targetId2 = targetInfo.targetId;

    TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
    if (targetType == TargetType::SPRITE &&
        targetId2 < _room->_sprites.size()) {
      auto &sprite = _room->_sprites[targetInfo.targetId];
      sprite._interactAction = static_cast<ROOMSpriteAction>(interactAction);
      if (interactAction == 1)
        sprite._cutsceneId = cutsceneId_;
    }
    break;
  }
  case CMD_SAVE_MENU:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SAVE_MENU");
#endif
    if (_cSaveMenu == nullptr)
      _cSaveMenu = std::make_unique<SaveMenu>(_room->_save.get(), _roomId);
    break;
  case CMD_MAX_HEALTH:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_MAX_HEALTH");
#endif
    _room->_save->hp = _room->_save->maxHp;
    _room->_ingame_menu->updateHp();
    if (_cBattle != nullptr)
      _cBattle->showHp();
    break;
  case CMD_CMP_ENEMY_HP: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_CMP_ENEMY_HP");
#endif
    u8 enemyIdx, comparator;
    u16 cmpValue;
    _commandData.read(&enemyIdx, 1);
    _commandData.read(&comparator, 1);
    _commandData.read(&cmpValue, 2);
    if (_cBattle == nullptr)
      break;
    if (enemyIdx >= _cBattle->_enemies.size())
      break;
    u16 flagValue = _cBattle->_enemies[enemyIdx]->_hp;
    if ((comparator & 3) == ComparisonOperator::EQUALS)
      _flag = (flagValue == cmpValue);
    else if ((comparator & 3) == ComparisonOperator::GREATER_THAN)
      _flag = (flagValue > cmpValue);
    else if ((comparator & 3) == ComparisonOperator::LESS_THAN)
      _flag = (flagValue < cmpValue);
    if (comparator & 4)
      _flag = !_flag;
    break;
  }
  case CMD_CLEAR_NAV_TASKS:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_CLEAR_NAV_TASKS");
#endif
    nav->clearAllTasks();
    break;
  case CMD_LOAD_SPRITE_RELATIVE: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_LOAD_SPRITE_RELATIVE");
#endif
    s32 dx, dy, layer;
    _commandData.read(&dx, 4);
    _commandData.read(&dy, 4);
    _commandData.read(&layer, 4);
    std::string path = _commandData.readstring();
    TargetInfo targetInfo = readTarget(_commandData);

    nav->spawn_relative(path, targetInfo, dx, dy, layer);
    break;
  }
  case CMD_SET_CELL:
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_SET_CELL");
#endif
    for (u8 &i : _room->_save->cell) {
      _commandData.read(&i, 1);
      if (i == 0)
        break;
    }
    break;
  case CMD_CLEAR: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_CLEAR");
#endif
    bool mainScreen;
    _commandData.read(&mainScreen, 1);
    Engine::TextBGManager &txt =
        mainScreen ? Engine::textMain : Engine::textSub;
    txt.clear();
    break;
  }
  case CMD_ENEMY_COMMAND: {
#ifdef DEBUG_CUTSCENES
    nocashMessage("CMD_ENEMY_COMMAND");
#endif
    s8 enemyNum;
    u8 enemyNum2;
    u8 enemyCmd;
    _commandData.read(&enemyNum, 1);
    _commandData.read(&enemyCmd, 1);

    if (_cBattle == nullptr) {
      nocashMessage("Attempted enemy command while not in battle!");
      break;
    }
    if (enemyNum < 0)
      enemyNum2 = _cBattle->_enemies.size() + enemyNum;
    else
      enemyNum2 = enemyNum;
    if (enemyNum2 >= _cBattle->_enemies.size()) {
      nocashMessage("Enemy command num outside of range!");
      break;
    }
    _cBattle->_enemies[enemyNum]->enemyCommand(enemyCmd);
    break;
  }
  default:
    std::string buffer = "Error cmd " + std::to_string(cmd) +
                         " unknown, pos:" + std::to_string(_commandData.tell());
    nocashMessage(buffer.c_str());
    return true;
  }
  return false;
}

