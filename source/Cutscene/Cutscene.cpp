//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"
#include "Battle/Battle.hpp"
#include "Battle/FlavorTextDialogue.hpp"
#include "ConditionalFile/RoomConditionalFile.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Cutscene/Navigation.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Engine.hpp"
#include "Fader.hpp"
#include "Formats/CSCN.hpp"
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

    if (!checkHeader())
      Engine::throw_("Error cutscene " + std::to_string(cutsceneId) + ": HEADER");
  } else {
    Engine::throw_("Error opening cutscene " + std::to_string(cutsceneId));
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
  u32 address;
  
  Navigation* nav;
  if (_cBattle)
    nav = &_cBattle->_nav;
  else
    nav = &_room->_nav;

  switch (cmd) {
  case CMD_DEBUG: {
    debug_cutscene("CMD_DEBUG");
    Engine::log_(_commandData.readstring());
    break;
  }
  case CMD_LOAD_SPRITE: {
    debug_cutscene("CMD_LOAD_SPRITE");
    u16 sprId;
    s32 x, y, layer;
    _commandData.read(&sprId, 2);
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    _commandData.read(&layer, 4);
    std::string path = _commandData.readstring();
    nav->spawn_sprite(sprId, path, x, y, layer);
    break;
  }
  case CMD_UNLOAD_SPRITE: {
    debug_cutscene("CMD_UNLOAD_SPRITE");
    u16 sprId;
    _commandData.read(&sprId, 2);
    nav->unload_sprite(sprId);
    break;
  }
  case CMD_PLAYER_CONTROL: {
    debug_cutscene("CMD_PLAYER_CONTROL");
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
    debug_cutscene("CMD_MANUAL_CAMERA");
    bool manualCamera;
    _commandData.read(&manualCamera, 1);
    _room->_camera._manual = manualCamera;
    break;
  }
  case CMD_WAIT: {
    debug_cutscene("CMD_WAIT");
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
    debug_cutscene("CMD_SET_SHOWN");
    TargetInfo targetInfo = readTarget(_commandData);
    bool shown;
    _commandData.read(&shown, 1);
    nav->set_shown(targetInfo, shown);
    break;
  }
  case CMD_SET_ANIMATION: {
    debug_cutscene("CMD_SET_ANIMATION");
    TargetInfo targetInfo = readTarget(_commandData);
    std::string anim = _commandData.readstring();
    nav->set_animation(targetInfo, anim);
    break;
  }
  case CMD_SET_OPACITY: {
    debug_cutscene("CMD_SET_OPACITY");
    TargetInfo targetInfo = readTarget(_commandData);
    u8 opacity;
    _commandData.read(&opacity, 1);
    nav->set_opacity(targetInfo, opacity);
    break;
  }
  case CMD_SET_POS: {
    debug_cutscene("CMD_SET_POS");
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    nav->set_position(targetInfo, x, y);
    break;
  }
  case CMD_MOVE: {
    debug_cutscene("CMD_MOVE");
    TargetInfo targetInfo = readTarget(_commandData);
    s32 dx, dy;
    _commandData.read(&dx, 4);
    _commandData.read(&dy, 4);
    nav->move(targetInfo, dx, dy);
    break;
  }
  case CMD_SET_SCALE: {
    debug_cutscene("CMD_SET_SCALE");
    TargetInfo targetInfo = readTarget(_commandData);
    s32 x, y;
    _commandData.read(&x, 4);
    _commandData.read(&y, 4);
    nav->set_scale(targetInfo, x, y);
    break;
  }
  case CMD_SET_POS_IN_FRAMES: {
    debug_cutscene("CMD_SET_POS_IN_FRAMES");
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
    debug_cutscene("CMD_MOVE_IN_FRAMES");
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
    debug_cutscene("CMD_SCALE_IN_FRAMES");
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
    debug_cutscene("CMD_START_DIALOGUE");
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
            _room->_save.get(), _cutsceneId, _room->_roomId, textId, target,
            std::move(targetIdle), std::move(targetTalk), std::move(typeSnd),
            std::move(font), framesPerLetter, txt, heartAlloc,
            std::move(speaker), x, y, std::move(speakerIdle),
            std::move(speakerTalk));
      else if (dialogue_type == DIALOGUE_LEFT_ALIGNED)
        _cDialogue = std::make_unique<DialogueLeftAligned>(
            _room->_save.get(), _cutsceneId, _room->_roomId, textId, target,
            targetIdle, targetTalk, typeSnd, font, framesPerLetter, txt,
            heartAlloc, x, y);
      else if (dialogue_type == DIALOGUE_FLAVOR_TEXT && _cBattle)
        _cDialogue = std::make_unique<FlavorTextDialogue>(
            _cBattle.get(), _cutsceneId, _room->_roomId, textId, typeSnd, font,
            framesPerLetter);
    }
    
    break;
  }
  case CMD_START_BATTLE: {
    debug_cutscene("CMD_START_BATTLE");
    if (_cBattle == nullptr) {
      _cBattle = std::make_unique<Battle>(this);
      _cBattle->loadFromBuffer(_commandData);
      _fader.startFade(FadeType::FADE_OUT);
    }
    return true;
  }
  case CMD_EXIT_BATTLE: {
    debug_cutscene("CMD_EXIT_BATTLE");
    bool battleWon = false;
    _commandData.read(&battleWon, 1);
    if (_cBattle != nullptr)
      _cBattle->exit(battleWon);
    return true;
  }
  case CMD_BATTLE_ATTACK: {
    debug_cutscene("CMD_BATTLE_ATTACK");
    if (_cBattle) // just in case
      _cBattle->startBattleAttacks();
      
    break;
  }
  case CMD_BATTLE_ACTION: {
    debug_cutscene("CMD_BATTLE_ACTION");
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
    debug_cutscene("CMD_JUMP_IF");
    _commandData.read(&address, 4);
    if (_flag)
      _commandData.seek(address);
    break;
  case CMD_JUMP_IF_NOT:
    debug_cutscene("CMD_JUMP_IF_NOT");
    _commandData.read(&address, 4);
    if (!_flag)
      _commandData.seek(address);
    break;
  case CMD_JUMP:
    debug_cutscene("CMD_JUMP");
    _commandData.read(&address, 4);
    _commandData.seek(address);
    break;
  case CMD_START_BGM: {
    debug_cutscene("CMD_START_BGM");
    bool loop;
    _commandData.read(&loop, 1);
    std::string path = _commandData.readstring();
    Audio2::playBGMusic(path, loop);
    break;
  }
  case CMD_STOP_BGM:
    debug_cutscene("CMD_STOP_BGM");
    Audio2::stopBGMusic();
    break;
  case CMD_PLAY_SFX: {
    debug_cutscene("CMD_PLAY_SFX");
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
    debug_cutscene("CMD_SET_FLAG");
    u16 flagId, flagValue;
    _commandData.read(&flagId, 2);
    _commandData.read(&flagValue, 2);
    _room->_save->flags[flagId] = flagValue;
    _room->_save->writePermanentFlags();
    break;
  }
  case CMD_ADD_FLAG: {
    debug_cutscene("CMD_ADD_FLAG");
    u16 flagId;
    s16 flagMod;
    _commandData.read(&flagId, 2);
    _commandData.read(&flagMod, 2);
    _room->_save->flags[flagId] += flagMod;
    _room->_save->writePermanentFlags();
    break;
  }
  case CMD_CMP_FLAG: {
    debug_cutscene("CMD_CMP_FLAG");
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
    debug_cutscene("CMD_SET_COLLIDER_ENABLED");
    u8 colliderId;
    bool enabled;
    _commandData.read(&colliderId, 1);
    _commandData.read(&enabled, 1);
    for (auto & collider : _room->_roomData._roomColliders) {
      if (collider._collId == colliderId) {
        collider._enabled = enabled;
        break;
      }
    }
    break;
  }
  case CMD_SET_ACTION: {
    debug_cutscene("CMD_SET_ACTION");
    // TODO: IMPROVE and add other actions
    u8 interactAction;
    u16 cutsceneId_;
    TargetInfo targetInfo = readTarget(_commandData);
    _commandData.read(&interactAction, 1);
    
    if (interactAction == 1)
      _commandData.read(&cutsceneId_, 2);

    TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
    if (targetType == TargetType::SPRITE) {
      auto it = _room->_sprites.find(targetInfo.targetId);
      if (it != _room->_sprites.end()) {
        auto &spr = it->second;
        spr._action = static_cast<RoomSpriteAction>(interactAction);
        if (interactAction == 1)
          spr._cutscene._cutscene_id = cutsceneId_;
      }
    }
    break;
  }
  case CMD_SAVE_MENU:
    debug_cutscene("CMD_SAVE_MENU");
    if (_cSaveMenu == nullptr)
      _cSaveMenu = std::make_unique<SaveMenu>(_room->_save.get(), _roomId);
    break;
  case CMD_MAX_HEALTH:
    debug_cutscene("CMD_MAX_HEALTH");
    _room->_save->hp = _room->_save->maxHp;
    _room->_ingame_menu->updateHp();
    if (_cBattle != nullptr)
      _cBattle->showHp();
    break;
  case CMD_CMP_ENEMY_HP: {
    debug_cutscene("CMD_CMP_ENEMY_HP");
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
    debug_cutscene("CMD_CLEAR_NAV_TASKS");
    nav->clearAllTasks();
    break;
  case CMD_LOAD_SPRITE_RELATIVE: {
    debug_cutscene("CMD_LOAD_SPRITE_RELATIVE");
    u16 sprId;
    s32 dx, dy, layer;
    _commandData.read(&sprId, 2);
    _commandData.read(&dx, 4);
    _commandData.read(&dy, 4);
    _commandData.read(&layer, 4);
    std::string path = _commandData.readstring();
    TargetInfo targetInfo = readTarget(_commandData);

    nav->spawn_relative(sprId, path, targetInfo, dx, dy, layer);
    break;
  }
  case CMD_SET_CELL:
    debug_cutscene("CMD_SET_CELL");
    for (u8 &i : _room->_save->cell) {
      _commandData.read(&i, 1);
      if (i == 0)
        break;
    }
    break;
  case CMD_CLEAR: {
    debug_cutscene("CMD_CLEAR");
    bool mainScreen;
    _commandData.read(&mainScreen, 1);
    Engine::TextBGManager &txt =
        mainScreen ? Engine::textMain : Engine::textSub;
    txt.clear();
    break;
  }
  case CMD_ENEMY_COMMAND: {
    debug_cutscene("CMD_ENEMY_COMMAND");
    s8 enemyNum;
    u8 enemyNum2;
    u8 enemyCmd;
    _commandData.read(&enemyNum, 1);
    _commandData.read(&enemyCmd, 1);

    if (_cBattle == nullptr) {
      Engine::throw_("Attempted enemy command while not in battle!");
      break;
    }
    if (enemyNum < 0)
      enemyNum2 = _cBattle->_enemies.size() + enemyNum;
    else
      enemyNum2 = enemyNum;
    if (enemyNum2 >= _cBattle->_enemies.size()) {
      Engine::throw_("Enemy command num outside of range!");
      break;
    }
    _cBattle->_enemies[enemyNum]->enemyCommand(enemyCmd);
    break;
  }
  default:
    Engine::throw_("Error cmd " + std::to_string(cmd) +
                   " unknown, pos:" + std::to_string(_commandData.tell()));
    return true;
  }
  return false;
}

