//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"

Cutscene* globalCutscene = nullptr;

Cutscene::Cutscene(uint16_t cutsceneId_) : cutsceneId(cutsceneId_) {
    char buffer[100];
    sprintf(buffer, "nitro:/data/cutscenes/cutscene%d.cscn", cutsceneId);
    FILE* f = fopen(buffer, "rb");
    if (f) {
        if (checkHeader(f)) {
            globalPlayer->playerControl = false;
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

    uint32_t version;
    fread(&version, 4, 1, f);

    if (version != 2) {
        return false;
    }

    uint32_t fileSize;

    fread(&fileSize, 4, 1, f);
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    uint32_t size = ftell(f);
    fseek(f, pos, SEEK_SET);

    if (size != fileSize) {
        return false;
    }

    return true;
}

void Cutscene::update(CutsceneLocation callingLocation) {
    waiting.update(callingLocation);
}

bool Cutscene::runCommands(CutsceneLocation callingLocation) {
    if (commandStream == nullptr)
        return true;
    if (waiting.getBusy())
        return false;
    if (ftell(commandStream) == commandStreamLen)
        return true;
    while (!waiting.getBusy() && ftell(commandStream) != commandStreamLen) {
        if (runCommand(callingLocation))
            break;
    }
    return false;
}

bool Cutscene::runCommand(CutsceneLocation callingLocation) {
    char buffer[100];
    uint8_t cmd;
    fread(&cmd, 1, 1, commandStream);
    int len;
    uint8_t targetType, targetId = 0, count;
    uint32_t address;
    Navigation* nav = nullptr;
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        nav = &globalRoom->nav;
    } else {
        return true;
    }

    switch (cmd) {
        case CMD_DEBUG:
            nocashMessage("CMD_DEBUG");
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            nocashMessage(buffer);
            break;
        case CMD_LOAD_SPRITE: {
            nocashMessage("CMD_LOAD_SPRITE");
            int32_t x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            Navigation::spawn_sprite(buffer, x, y, callingLocation);
            break;
        }
        case CMD_UNLOAD_SPRITE: {
            nocashMessage("CMD_UNLOAD_SPRITE");
            uint8_t sprId;
            fread(&sprId, 1, 1, commandStream);
            Navigation::unload_sprite(sprId, callingLocation);
            break;
        }
        case CMD_PLAYER_CONTROL: {
            nocashMessage("CMD_PLAYER_CONTROL");
            bool playerControl;
            fread(&playerControl, 1, 1, commandStream);
            globalPlayer->playerControl = playerControl;
            break;
        }
        case CMD_MANUAL_CAMERA: {
            nocashMessage("CMD_MANUAL_CAMERA");
            bool manualCamera;
            fread(&manualCamera, 1, 1, commandStream);
            globalCamera.manual = manualCamera;
            break;
        }
        case CMD_WAIT_EXIT:
            nocashMessage("CMD_WAIT_EXIT");
            waiting.waitExit();
            break;
        case CMD_WAIT_ENTER:
            nocashMessage("CMD_WAIT_ENTER");
            waiting.waitEnter();
            break;
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
        case CMD_WAIT_FRAMES: {
            nocashMessage("CMD_WAIT_FRAMES");
            uint16_t frameCount;
            fread(&frameCount, 2, 1, commandStream);
            waiting.waitFrames(frameCount);
            break;
        }
        case CMD_SET_POS: {
            nocashMessage("CMD_SET_POS");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            int32_t x, y;
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
            int32_t x, y;
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
            int32_t x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            uint16_t frames;
            fread(&frames, 2, 1, commandStream);
            nav->set_pos_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_MOVE_IN_FRAMES: {
            nocashMessage("CMD_MOVE_IN_FRAMES");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            int32_t x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            uint16_t frames;
            fread(&frames, 2, 1, commandStream);
            nav->move_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_SCALE_IN_FRAMES: {
            nocashMessage("CMD_SCALE_IN_FRAMES");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            int32_t x, y;
            fread(&x, 4, 1, commandStream);
            fread(&y, 4, 1, commandStream);
            uint16_t frames;
            fread(&frames, 2, 1, commandStream);
            nav->scale_in_frames(targetType, targetId, x, y, frames, callingLocation);
            break;
        }
        case CMD_START_DIALOGUE: {
            nocashMessage("CMD_START_DIALOGUE");
            uint16_t textId, framesPerLetter;
            int32_t x, y;
            char speaker[50], font[50];
            char idleAnim[50], talkAnim[50];
            char idleAnim2[50], talkAnim2[50];

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
            fread(font, len + 1, 1, commandStream);

            fread(&framesPerLetter, 2, 1, commandStream);

            Engine::SpriteManager* target = Navigation::getTarget(targetType, targetId, callingLocation);
            if (currentDialogue == nullptr) {
                currentDialogue = new Dialogue(textId, speaker, x, y, idleAnim, talkAnim,
                                               target, idleAnim2, talkAnim2, font, framesPerLetter);
            }
            break;
        }
        case CMD_WAIT_DIALOGUE_END:
            nocashMessage("CMD_WAIT_DIALOGUE_END");
            waiting.waitDialogueEnd();
            break;
        case CMD_START_BATTLE:
            nocashMessage("CMD_START_BATTLE");
            break;
        case CMD_EXIT_BATTLE:
            nocashMessage("CMD_EXIT_BATTLE");
            break;
        case CMD_START_BATTLE_DIALOGUE:
            nocashMessage("CMD_START_BATTLE_DIALOGUE");
            fread(buffer, 4, 1, commandStream);
            fread(buffer, 4, 1, commandStream);
            fread(buffer, 2, 1, commandStream);
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(buffer, 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            fread(buffer, 2, 1, commandStream);
            break;
        case CMD_BATTLE_ATTACK:
            nocashMessage("CMD_BATTLE_ATTACK");
            fread(buffer, 2, 1, commandStream);
            break;
        case CMD_WAIT_BATTLE_ATTACK:
            nocashMessage("CMD_WAIT_BATTLE_ATTACK");
            break;
        case CMD_WAIT_BATTLE_ACTION:
            nocashMessage("CMD_WAIT_BATTLE_ACTION");
            fread(&count, 1, 1, commandStream);
            for (int i = 0; i < count; i++) {
                len = strlen_file(commandStream, 0);
                fread(buffer, len + 1, 1, commandStream);
            }
            break;
        case CMD_CMP_BATTLE_ACTION:
            nocashMessage("CMD_CMP_BATTLE_ACTION");
            fread(buffer, 1, 1, commandStream);
            break;
        case CMD_CHECK_HIT:
            nocashMessage("CMD_CHECK_HIT");
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

            BGM::globalWAV.loadWAV(buffer);
            BGM::globalWAV.setLoop(loop);
            BGM::playWAV(BGM::globalWAV);
            break;
        }
        case CMD_STOP_BGM:
            nocashMessage("CMD_STOP_BGM");
            BGM::stopWAV();
            break;
        case CMD_SET_FLAG: {
            nocashMessage("CMD_SET_FLAG");
            uint16_t flagId, flagValue;
            fread(&flagId, 2, 1, commandStream);
            fread(&flagValue, 2, 1, commandStream);
            saveGlobal.flags[flagId] = flagValue;
            break;
        }
        case CMD_CMP_FLAG: {
            nocashMessage("CMD_SET_FLAG");
            uint16_t flagId, flagValue, cmpValue;
            uint8_t comparator;
            fread(&flagId, 2, 1, commandStream);
            fread(&comparator, 1, 1, commandStream);
            fread(&cmpValue, 2, 1, commandStream);
            flagValue = saveGlobal.flags[flagId];
            if (comparator == ComparisonOperator::EQUALS)
                flag = (flagValue == cmpValue);
            else if (comparator == ComparisonOperator::GREATER_THAN)
                flag = (flagValue > cmpValue);
            else if (comparator == ComparisonOperator::LESS_THAN)
                flag = (flagValue < cmpValue);
            break;
        }
        case CMD_SET_COLLIDER_ENABLED: {
            uint8_t colliderId;
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
        default:
            sprintf(buffer, "Error cmd %d unknown pos: %ld", cmd, ftell(commandStream));
            nocashMessage(buffer);
            fclose(commandStream);
            commandStream = nullptr;
            return true;
    }
    return false;
}