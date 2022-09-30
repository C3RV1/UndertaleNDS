//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"
#include "Cutscene/Navigation.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Engine/Engine.hpp"
#include "Save.hpp"
#include "Battle/Battle.hpp"
#include "Battle/BattleAttack.hpp"
#include "Room/Room.hpp"
#include "Formats/utils.hpp"
#include "Room/Player.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Camera.hpp"

Cutscene* globalCutscene = nullptr;

Cutscene::Cutscene(u16 cutsceneId_) : cutsceneId(cutsceneId_) {
    char buffer[100];
    roomId = globalRoom->roomId;
    sprintf(buffer, "nitro:/data/cutscenes/r%d/c%d.cscn", roomId, cutsceneId);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        if (checkHeader(f)) {
            long pos = ftell(f);
            fseek(f, 0, SEEK_END);
            commandStreamLen = ftell(f);
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
    commandStream = f;
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

    if (version != 5) {
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
    if (cDialogue != nullptr) {
        if (cDialogue->update()) {
            cDialogue->free_();
            delete cDialogue;
            cDialogue = nullptr;
        }
    } else if (cSaveMenu != nullptr) {
        if (cSaveMenu->update()) {
            cSaveMenu->free_();
            delete cSaveMenu;
            cSaveMenu = nullptr;
        }
    }
}

bool Cutscene::runCommands(CutsceneLocation callingLocation) {
    waiting.update(callingLocation, true);
    if (commandStream == nullptr)
        return true;
    if (waiting.getBusy())
        return false;
    if (ftell(commandStream) == commandStreamLen)
        return true;
    while (!waiting.getBusy() && ftell(commandStream) != commandStreamLen) {
        if (runCommand(callingLocation))
            break;
        waiting.update(callingLocation, false);
    }
    return false;
}

bool Cutscene::runCommand(CutsceneLocation callingLocation) {
    char buffer[100];
    u8 cmd;
    fread(&cmd, 1, 1, commandStream);
    int len;
    u8 targetType, targetId = 0;
    u32 address;
    Navigation* nav;
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        nav = &globalRoom->nav;
    } else {
        nav = &globalBattle->nav;
    }

    switch (cmd) {
        case CMD_DEBUG:
            nocashMessage("CMD_DEBUG");
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            nocashMessage(buffer);
            break;
        case CMD_LOAD_TEXTURE:
            nocashMessage("CMD_LOAD_TEXTURE");
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            Navigation::load_texture(buffer, callingLocation);
            break;
        case CMD_UNLOAD_TEXTURE:
            nocashMessage("CMD_LOAD_TEXTURE");
            fread(&targetId, 1, 1, commandStream);
            Navigation::unload_texture(targetId, callingLocation);
            break;
        case CMD_LOAD_SPRITE: {
            nocashMessage("CMD_LOAD_SPRITE");
            s32 x, y, layer;
            u8 texId;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            fread(&layer, 4, 1, commandStream);
            fread(&texId, 1, 1, commandStream);
            Navigation::spawn_sprite(texId, x, y, layer, callingLocation);
            break;
        }
        case CMD_UNLOAD_SPRITE: {
            nocashMessage("CMD_UNLOAD_SPRITE");
            u8 sprId;
            fread(&sprId, 1, 1, commandStream);
            Navigation::unload_sprite(sprId, callingLocation);
            break;
        }
        case CMD_PLAYER_CONTROL: {
            nocashMessage("CMD_PLAYER_CONTROL");
            bool playerControl;
            fread(&playerControl, 1, 1, commandStream);
            globalPlayer->playerControl = playerControl;
            if (playerControl)
                globalInGameMenu.show(false);
            else
                globalInGameMenu.hide();
            break;
        }
        case CMD_MANUAL_CAMERA: {
            nocashMessage("CMD_MANUAL_CAMERA");
            bool manualCamera;
            fread(&manualCamera, 1, 1, commandStream);
            globalCamera.manual = manualCamera;
            break;
        }
        case CMD_WAIT: {
            nocashMessage("CMD_WAIT");
            u8 waitType;
            fread(&waitType, 1, 1, commandStream);
            if (waitType == WAIT_FRAMES) {
                u16 frames;
                fread(&frames, 2, 1, commandStream);
                waiting.waitFrames(frames);
                break;
            }
            waiting.wait((WaitingType)(waitType));
            break;
        }
        case CMD_SET_SHOWN: {
            nocashMessage("CMD_SET_SHOWN");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            bool shown;
            fread(&shown, 1, 1, commandStream);
            Navigation::set_shown(targetType, targetId, shown, callingLocation);
            break;
        }
        case CMD_SET_ANIMATION:
            nocashMessage("CMD_SET_ANIMATION");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            Navigation::set_animation(targetType, targetId, buffer, callingLocation);
            break;
        case CMD_SET_POS: {
            nocashMessage("CMD_SET_POS");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            s32 x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            Navigation::set_position(targetType, targetId, x, y, callingLocation);
            break;
        }
        case CMD_SET_SCALE: {
            nocashMessage("CMD_SET_SCALE");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            s32 x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            Navigation::set_scale(targetType, targetId, x, y, callingLocation);
            break;
        }
        case CMD_SET_POS_IN_FRAMES: {
            nocashMessage("CMD_SET_POS_IN_FRAMES");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            s32 x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            u16 frames;
            fread(&frames, 2, 1, commandStream);
            nav->set_pos_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_MOVE_IN_FRAMES: {
            nocashMessage("CMD_MOVE_IN_FRAMES");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            s32 x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            u16 frames;
            fread(&frames, 2, 1, commandStream);
            nav->move_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_SCALE_IN_FRAMES: {
            nocashMessage("CMD_SCALE_IN_FRAMES");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            s32 x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            u16 frames;
            fread(&frames, 2, 1, commandStream);
            nav->scale_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_START_DIALOGUE: {
            nocashMessage("CMD_START_DIALOGUE");
            u16 textId, framesPerLetter;
            s32 x, y;
            char speaker[50], font[50];
            char idleAnim[50], talkAnim[50];
            char idleAnim2[50], talkAnim2[50];
            char typeSnd[50];

            fread(&textId, 2, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(speaker, len + 1, 1, commandStream);

            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(idleAnim, len + 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(talkAnim, len + 1, 1, commandStream);

            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(idleAnim2, len + 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(talkAnim2, len + 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(typeSnd, len + 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(font, len + 1, 1, commandStream);

            fread(&framesPerLetter, 2, 1, commandStream);

            Engine::Sprite* target = Navigation::getTarget(targetType, targetId, callingLocation);
            if (cDialogue == nullptr) {
                bool isRoom = callingLocation == ROOM || callingLocation == LOAD_ROOM;
                cDialogue = new Dialogue(isRoom, textId, speaker, x, y, idleAnim, talkAnim,
                                         target, idleAnim2, talkAnim2, typeSnd,
                                         font, framesPerLetter);
            }
            break;
        }
        case CMD_START_BATTLE: {
            nocashMessage("CMD_START_BATTLE");
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM)
                runBattle(commandStream);
            return true;
        }
        case CMD_EXIT_BATTLE: {
            nocashMessage("CMD_EXIT_BATTLE");
            if (globalBattle != nullptr)
                globalBattle->running = false;
            return true;
        }
        case CMD_BATTLE_ATTACK: {
            nocashMessage("CMD_BATTLE_ATTACK");
            u16 attackId;
            fread(&attackId, 2, 1, commandStream);
            if (globalBattle != nullptr) {  // just in case
                globalBattle->resetBattleAttack();
                globalBattle->currentBattleAttack = getBattleAttack(attackId);
            }
            break;
        }
        case CMD_CMP_BATTLE_ACTION:
            nocashMessage("CMD_CMP_BATTLE_ACTION");
            fread(buffer, 1, 1, commandStream);
            break;
        case CMD_CHECK_HIT:
            nocashMessage("CMD_CHECK_HIT");
            if (callingLocation == BATTLE || callingLocation == LOAD_BATTLE)
                flag = globalBattle->hitFlag;
            break;
        case CMD_JUMP_IF:
            nocashMessage("CMD_JUMP_IF");
            fread(&address, 4, 1, commandStream);
            if (flag)
                fseek(commandStream, address, SEEK_SET);
            break;
        case CMD_JUMP_IF_NOT:
            nocashMessage("CMD_JUMP_IF_NOT");
            fread(&address, 4, 1, commandStream);
            if (!flag)
                fseek(commandStream, address, SEEK_SET);
            break;
        case CMD_JUMP:
            nocashMessage("CMD_JUMP");
            fread(&address, 4, 1, commandStream);
            fseek(commandStream, address, SEEK_SET);
            break;
        case CMD_START_BGM: {
            nocashMessage("CMD_START_BGM");
            bool loop;
            fread(&loop, 1, 1, commandStream);

            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            Audio::playBGMusic(buffer, loop);
            break;
        }
        case CMD_STOP_BGM:
            nocashMessage("CMD_STOP_BGM");
            Audio::stopBGMusic();
            break;
        case CMD_PLAY_SFX: {
            nocashMessage("CMD_PLAY_SFX");
            s8 loops;
            fread(&loops, 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);

            auto *sfxWav = new Audio::WAV;
            sfxWav->deleteOnStop = true;
            sfxWav->loadWAV(buffer);
            sfxWav->setLoops(loops);
            sfxWav->play();
            break;
        }
        case CMD_SET_FLAG: {
            nocashMessage("CMD_SET_FLAG");
            u16 flagId, flagValue;
            fread(&flagId, 2, 1, commandStream);
            fread(&flagValue, 2, 1, commandStream);
            globalSave.flags[flagId] = flagValue;
            break;
        }
        case CMD_MOD_FLAG: {
            nocashMessage("CMD_MOD_FLAG");
            u16 flagId;
            s16 flagMod;
            fread(&flagId, 2, 1, commandStream);
            fread(&flagMod, 2, 1, commandStream);
            globalSave.flags[flagId] += flagMod;
            break;
        }
        case CMD_CMP_FLAG: {
            nocashMessage("CMD_SET_FLAG");
            u16 flagId, flagValue, cmpValue;
            u8 comparator;
            fread(&flagId, 2, 1, commandStream);
            fread(&comparator, 1, 1, commandStream);
            fread(&cmpValue, 2, 1, commandStream);
            flagValue = globalSave.flags[flagId];
            if ((comparator & 3) == ComparisonOperator::EQUALS)
                flag = (flagValue == cmpValue);
            else if ((comparator & 3) == ComparisonOperator::GREATER_THAN)
                flag = (flagValue > cmpValue);
            else if ((comparator & 3) == ComparisonOperator::LESS_THAN)
                flag = (flagValue < cmpValue);
            if (comparator & 4)
                flag = !flag;
            break;
        }
        case CMD_SET_COLLIDER_ENABLED: {
            u8 colliderId;
            bool enabled;
            fread(&colliderId, 1, 1, commandStream);
            fread(&enabled, 1, 1, commandStream);
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
                if (colliderId < globalRoom->roomData.roomColliders.colliderCount) {
                    globalRoom->roomData.roomColliders.roomColliders[colliderId].enabled = enabled;
                }
            }
            break;
        }
        case CMD_SET_INTERACT_ACTION: {
            nocashMessage("CMD_SET_INTERACT_ACTION");
            u8 interactAction;
            u16 cutsceneId_;
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            fread(&interactAction, 1, 1, commandStream);
            if (interactAction == 1)
                fread(&cutsceneId_, 2, 1, commandStream);
            if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
                if (targetType == TargetType::SPRITE && targetId < globalRoom->spriteCount) {
                    ManagedSprite* sprite = globalRoom->sprites[targetId];
                    sprite->interactAction = interactAction;
                    if (interactAction == 1)
                        sprite->cutsceneId = cutsceneId_;
                }
            }
            break;
        }
        case CMD_SAVE_MENU:
            nocashMessage("CMD_SAVE_MENU");
            if (cSaveMenu == nullptr)
                cSaveMenu = new SaveMenu();
            break;
        case CMD_MAX_HEALTH:
            nocashMessage("CMD_MAX_HEALTH");
            globalSave.hp = globalSave.maxHp;
            break;
        default:
            sprintf(buffer, "Error cmd %d unknown pos: %ld", cmd, ftell(commandStream));
            nocashMessage(buffer);
            fclose(commandStream);
            commandStream = nullptr;
            return true;
    }
    return false;
}

Cutscene::~Cutscene() {
    if (commandStream != nullptr)
        fclose(commandStream);
    if (cDialogue != nullptr) {
        cDialogue->free_();
        delete cDialogue;
        cDialogue = nullptr;
    }
}