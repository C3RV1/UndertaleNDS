//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"
#include "Cutscene/Navigation.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Save.hpp"
#include "Battle/Battle.hpp"
#include "Battle/BattleAttack.hpp"
#include "Room/Room.hpp"
#include "Formats/CSCN.hpp"
#include "Formats/utils.hpp"
#include "Room/Player.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Camera.hpp"
#include "DEBUG_FLAGS.hpp"
#include <memory>

std::unique_ptr<Cutscene> globalCutscene = nullptr;

Cutscene::Cutscene(u16 cutsceneId, u16 roomId) : _cutsceneId(cutsceneId), _roomId(roomId) {
    char buffer[100];
    sprintf(buffer, "nitro:/data/cutscenes/r%d/c%d.cscn", roomId, cutsceneId);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        if (checkHeader(f)) {
            long pos = ftell(f);
            fseek(f, 0, SEEK_END);
            _commandStreamLen = ftell(f);
            fseek(f, pos, SEEK_SET);
        }
        else {
            sprintf(buffer, "Error cutscene %d header", cutsceneId);
            nocashMessage(buffer);
            fclose(f);
            f = nullptr;
        }
    }
    else {
        sprintf(buffer, "Error opening cutscene %d", cutsceneId);
        nocashMessage(buffer);
    }
    _commandStream = f;
}

bool Cutscene::checkHeader(FILE *f) {
    char header[4];
    char expectedHeader[4] = {'C', 'S', 'C', 'N'};

    fread(header, 4, 1, f);
    if (memcmp(header, expectedHeader, 4) != 0) {
        return false;
    }

    u32 version;
    fread(&version, 4, 1, f);

    if (version != CSCN::version) {
        return false;
    }

    u32 fileSize;

    fread(&fileSize, 4, 1, f);
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    u32 size = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (size != fileSize) {
        return false;
    }

    return true;
}

void Cutscene::update() {
    if (_cDialogue != nullptr) {
        if (_cDialogue->update()) {
            _cDialogue = nullptr;
        }
    } else if (_cSaveMenu != nullptr) {
        if (_cSaveMenu->update()) {
            _cSaveMenu = nullptr;
        }
    }
}

bool Cutscene::runCommands(CutsceneLocation callingLocation) {
    _waiting.update(callingLocation, true);
    if (_commandStream == nullptr)
        return true;
    if (_waiting.getBusy())
        return false;
    if (ftell(_commandStream) >= _commandStreamLen)
        return true;
    while (!_waiting.getBusy() && ftell(_commandStream) < _commandStreamLen) {
        if (runCommand(callingLocation))
            break;
        _waiting.update(callingLocation, false);
    }
    return false;
}

bool Cutscene::runCommand(CutsceneLocation callingLocation) {
    char buffer[100];
    u8 cmd;
    fread(&cmd, 1, 1, _commandStream);
    int len;
    u8 targetType;
    s8 targetId = 0;
    u32 address;
    Navigation* nav;
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        nav = &globalRoom->_nav;
    } else {
        nav = &globalBattle->_nav;
    }

    switch (cmd) {
        case CMD_DEBUG:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_DEBUG");
#endif
            len = str_len_file(_commandStream, 0);
            fread(buffer, len + 1, 1, _commandStream);
            nocashMessage(buffer);
            break;
        case CMD_LOAD_TEXTURE:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_LOAD_TEXTURE");
#endif
            len = str_len_file(_commandStream, 0);
            fread(buffer, len + 1, 1, _commandStream);
            Navigation::load_texture(buffer, callingLocation);
            break;
        case CMD_UNLOAD_TEXTURE:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_LOAD_TEXTURE");
#endif
            fread(&targetId, 1, 1, _commandStream);
            Navigation::unload_texture(targetId, callingLocation);
            break;
        case CMD_LOAD_SPRITE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_LOAD_SPRITE");
#endif
            s32 x, y, layer;
            s8 texId;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            fread(&layer, 4, 1, _commandStream);
            fread(&texId, 1, 1, _commandStream);
            Navigation::spawn_sprite(texId, x, y, layer, callingLocation);
            break;
        }
        case CMD_UNLOAD_SPRITE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_UNLOAD_SPRITE");
#endif
            s8 sprId;
            fread(&sprId, 1, 1, _commandStream);
            Navigation::unload_sprite(sprId, callingLocation);
            break;
        }
        case CMD_PLAYER_CONTROL: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_PLAYER_CONTROL");
#endif
            bool playerControl;
            fread(&playerControl, 1, 1, _commandStream);
            globalPlayer->setPlayerControl(playerControl);
            if (playerControl)
                globalInGameMenu.show(false);
            else
                globalInGameMenu.hide();
            break;
        }
        case CMD_MANUAL_CAMERA: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_MANUAL_CAMERA");
#endif
            bool manualCamera;
            fread(&manualCamera, 1, 1, _commandStream);
            globalCamera._manual = manualCamera;
            break;
        }
        case CMD_WAIT: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_WAIT");
#endif
            u8 waitType;
            fread(&waitType, 1, 1, _commandStream);
            if (waitType == WAIT_FRAMES) {
                u16 frames;
                fread(&frames, 2, 1, _commandStream);
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
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            bool shown;
            fread(&shown, 1, 1, _commandStream);
            Navigation::set_shown(targetType, targetId, shown, callingLocation);
            break;
        }
        case CMD_SET_ANIMATION:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_ANIMATION");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            len = str_len_file(_commandStream, 0);
            fread(buffer, len + 1, 1, _commandStream);
            Navigation::set_animation(targetType, targetId, buffer, callingLocation);
            break;
        case CMD_SET_POS: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_POS");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 x, y;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            Navigation::set_position(targetType, targetId, x, y, callingLocation);
            break;
        }
        case CMD_MOVE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_MOVE");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 dx, dy;
            fread(&dx, 4, 1, _commandStream);
            fread(&dy, 4, 1, _commandStream);
            Navigation::move(targetType, targetId, dx, dy, callingLocation);
            break;
        }
        case CMD_SET_SCALE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_SCALE");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 x, y;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            Navigation::set_scale(targetType, targetId, x, y, callingLocation);
            break;
        }
        case CMD_SET_POS_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_POS_IN_FRAMES");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 x, y;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            u16 frames;
            fread(&frames, 2, 1, _commandStream);
            nav->set_pos_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_MOVE_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_MOVE_IN_FRAMES");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 x, y;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            u16 frames;
            fread(&frames, 2, 1, _commandStream);
            nav->move_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_SCALE_IN_FRAMES: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SCALE_IN_FRAMES");
#endif
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            s32 x, y;
            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);
            u16 frames;
            fread(&frames, 2, 1, _commandStream);
            nav->scale_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_START_DIALOGUE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_START_DIALOGUE");
#endif
            u16 textId, framesPerLetter;
            s32 x, y;
            char speaker[50], font[50];
            char speakerIdle[50], speakerTalk[50];
            char targetIdle[50], targetTalk[50];
            char typeSnd[50];
            bool mainScreen;
            DialogueType dialogue_type;

            fread(&dialogue_type, 1, 1, _commandStream);
            fread(&textId, 2, 1, _commandStream);

            if (dialogue_type == DIALOGUE_CENTERED) {
                len = str_len_file(_commandStream, 0);
                fread(speaker, len + 1, 1, _commandStream);
            }

            fread(&x, 4, 1, _commandStream);
            fread(&y, 4, 1, _commandStream);

            if (dialogue_type == DIALOGUE_CENTERED) {
                len = str_len_file(_commandStream, 0);
                fread(speakerIdle, len + 1, 1, _commandStream);

                len = str_len_file(_commandStream, 0);
                fread(speakerTalk, len + 1, 1, _commandStream);
            }

            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);

            len = str_len_file(_commandStream, 0);
            fread(targetIdle, len + 1, 1, _commandStream);

            len = str_len_file(_commandStream, 0);
            fread(targetTalk, len + 1, 1, _commandStream);

            len = str_len_file(_commandStream, 0);
            fread(typeSnd, len + 1, 1, _commandStream);

            len = str_len_file(_commandStream, 0);
            fread(font, len + 1, 1, _commandStream);

            fread(&framesPerLetter, 2, 1, _commandStream);
            fread(&mainScreen, 1, 1, _commandStream);
            Engine::TextBGManager& txt = mainScreen ? Engine::textMain : Engine::textSub;
            Engine::AllocationMode heartAlloc = mainScreen ? Engine::Allocated3D : Engine::AllocatedOAM;

            Engine::Sprite* target = Navigation::getTarget(targetType, targetId, callingLocation);
            if (_cDialogue == nullptr) {
                if (dialogue_type == DIALOGUE_CENTERED)
                    _cDialogue = std::make_unique<DialogueCentered>(
                            textId, speaker, x, y, speakerIdle,
                            speakerTalk, target, targetIdle, targetTalk,
                            typeSnd, font, framesPerLetter, txt, heartAlloc
                            );
                else if (dialogue_type == DIALOGUE_LEFT_ALIGNED)
                    _cDialogue = std::make_unique<DialogueLeftAligned>(
                        textId, x, y, target, targetIdle, targetTalk,
                        typeSnd, font, framesPerLetter, txt, heartAlloc);
            }
            break;
        }
        case CMD_START_BATTLE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_START_BATTLE");
#endif
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM)
                runBattle(_commandStream);
            return true;
        }
        case CMD_EXIT_BATTLE: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_EXIT_BATTLE");
#endif
            bool battleWon = false;
            fread(&battleWon, 1, 1, _commandStream);
            if (globalBattle != nullptr)
                globalBattle->exit(battleWon);
            return true;
        }
        case CMD_BATTLE_ATTACK: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_BATTLE_ATTACK");
#endif
            if (globalBattle != nullptr) {  // just in case
                globalBattle->startBattleAttacks();
            }
            break;
        }
        case CMD_BATTLE_ACTION: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_BATTLE_ACTION");
#endif
            if (globalBattle == nullptr) // just in case
                break;
            if (globalBattle->_cBattleAction != nullptr)
                break;
            globalBattle->hide();

            s16 flavorTextId;
            fread(&flavorTextId, 2, 1, _commandStream);

            globalBattle->_cBattleAction = std::make_unique<BattleAction>(
                    &globalBattle->_enemies, flavorTextId);
            break;
        }
        case CMD_CHECK_HIT:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_CHECK_HIT");
#endif
            if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE)
                _flag = globalBattle->_hitFlag;
            break;
        case CMD_JUMP_IF:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_JUMP_IF");
#endif
            fread(&address, 4, 1, _commandStream);
            if (_flag)
                fseek(_commandStream, address, SEEK_SET);
            break;
        case CMD_JUMP_IF_NOT:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_JUMP_IF_NOT");
#endif
            fread(&address, 4, 1, _commandStream);
            if (!_flag)
                fseek(_commandStream, address, SEEK_SET);
            break;
        case CMD_JUMP:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_JUMP");
#endif
            fread(&address, 4, 1, _commandStream);
            fseek(_commandStream, address, SEEK_SET);
            break;
        case CMD_START_BGM: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_START_BGM");
#endif
            bool loop;
            fread(&loop, 1, 1, _commandStream);

            len = str_len_file(_commandStream, 0);
            fread(buffer, len + 1, 1, _commandStream);
            Audio2::playBGMusic(buffer, loop);
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
            fread(&loops, 1, 1, _commandStream);
            len = str_len_file(_commandStream, 0);
            fread(buffer, len + 1, 1, _commandStream);

            auto sfxWav = std::make_shared<Audio2::WAV>();
            sfxWav->freeOnStop(sfxWav);
            sfxWav->loadWAV(buffer);
            sfxWav->setLoops(loops);
            sfxWav->play();
            break;
        }
        case CMD_SET_FLAG: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_FLAG");
#endif
            u16 flagId, flagValue;
            fread(&flagId, 2, 1, _commandStream);
            fread(&flagValue, 2, 1, _commandStream);
            globalSave.flags[flagId] = flagValue;
            globalSave.writePermanentFlags();
            break;
        }
        case CMD_MOD_FLAG: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_MOD_FLAG");
#endif
            u16 flagId;
            s16 flagMod;
            fread(&flagId, 2, 1, _commandStream);
            fread(&flagMod, 2, 1, _commandStream);
            globalSave.flags[flagId] += flagMod;
            globalSave.writePermanentFlags();
            break;
        }
        case CMD_CMP_FLAG: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_CMP_FLAG");
#endif
            u16 flagId, flagValue, cmpValue;
            u8 comparator;
            fread(&flagId, 2, 1, _commandStream);
            fread(&comparator, 1, 1, _commandStream);
            fread(&cmpValue, 2, 1, _commandStream);
            flagValue = globalSave.flags[flagId];
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
            fread(&colliderId, 1, 1, _commandStream);
            fread(&enabled, 1, 1, _commandStream);
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
                if (colliderId < globalRoom->_roomData.roomColliders.roomColliders.size()) {
                    globalRoom->_roomData.roomColliders.roomColliders[colliderId].enabled = enabled;
                }
            }
            break;
        }
        case CMD_SET_ACTION: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_ACTION");
#endif
            u8 interactAction;
            u16 cutsceneId_;
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);
            fread(&interactAction, 1, 1, _commandStream);
            if (interactAction == 1)
                fread(&cutsceneId_, 2, 1, _commandStream);
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
                if (targetType == TargetType::SPRITE && targetId < globalRoom->_sprites.size()) {
                    auto & sprite = globalRoom->_sprites[targetId];
                    sprite->_interactAction = interactAction;
                    if (interactAction == 1)
                        sprite->_cutsceneId = cutsceneId_;
                }
            }
            break;
        }
        case CMD_SAVE_MENU:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SAVE_MENU");
#endif
            if (_cSaveMenu == nullptr)
                _cSaveMenu = std::make_unique<SaveMenu>();
            break;
        case CMD_MAX_HEALTH:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_MAX_HEALTH");
#endif
            globalSave.hp = globalSave.maxHp;
            break;
        case CMD_SET_ENEMY_ATTACK: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_ENEMY_ATTACK");
#endif
            u8 enemyIdx;
            u16 attackId;
            fread(&enemyIdx, 1, 1, _commandStream);
            fread(&attackId, 2, 1, _commandStream);
            if (globalBattle != nullptr) {
                if (enemyIdx < globalBattle->_enemies.size()) {
                    globalBattle->_enemies[enemyIdx]._attackId = attackId;
                }
            }
            break;
        }
        case CMD_CMP_ENEMY_HP: {
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_CMP_ENEMY_HP");
#endif
            u8 enemyIdx, comparator;
            u16 cmpValue;
            fread(&enemyIdx, 1, 1, _commandStream);
            fread(&comparator, 1, 1, _commandStream);
            fread(&cmpValue, 2, 1, _commandStream);
            if (globalBattle == nullptr)
                break;
            if (enemyIdx >= globalBattle->_enemies.size())
                break;
            u16 flagValue = globalBattle->_enemies[enemyIdx]._hp;
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
            s8 texId;
            fread(&dx, 4, 1, _commandStream);
            fread(&dy, 4, 1, _commandStream);
            fread(&layer, 4, 1, _commandStream);
            fread(&texId, 1, 1, _commandStream);
            fread(&targetType, 1, 1, _commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, _commandStream);

            Navigation::spawn_relative(texId, targetType, targetId, dx, dy, layer,
                                       callingLocation);
            break;
        }
        case CMD_SET_CELL:
#ifdef DEBUG_CUTSCENES
            nocashMessage("CMD_SET_CELL");
#endif
            for(u8 &i : globalSave.cell) {
                fread(&i, 1, 1, _commandStream);
                if (i == 0)
                    break;
            }
            break;
        default:
            sprintf(buffer, "Error cmd %d unknown, pos: %ld", cmd, ftell(_commandStream));
            nocashMessage(buffer);
            fclose(_commandStream);
            _commandStream = nullptr;
            return true;
    }
    return false;
}

Cutscene::~Cutscene() {
    if (_commandStream != nullptr)
        fclose(_commandStream);
}