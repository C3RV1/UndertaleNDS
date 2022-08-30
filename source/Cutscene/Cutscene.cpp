//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Cutscene.hpp"

Cutscene* globalCutscene = nullptr;

Cutscene::Cutscene(uint16_t cutsceneId) {
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

    if (version != 1) {
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
    while (!waiting.getBusy() && ftell(commandStream) != commandStreamLen) {
        if (runCommand(callingLocation))
            break;
    }
    if (ftell(commandStream) == commandStreamLen)
        return true;
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
        case CMD_LOAD_SPRITE:
            nocashMessage("CMD_LOAD_SPRITE");
            fread(buffer, 4, 1, commandStream);
            fread(buffer, 4, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            break;
        case CMD_UNLOAD_SPRITE:
            nocashMessage("CMD_UNLOAD_SPRITE");
            fread(buffer, 1, 1, commandStream);
            break;
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
        case CMD_WAIT_LOAD:
            nocashMessage("CMD_WAIT_LOAD");
            waiting.waitLoad();
            break;
        case CMD_SHOW:
            nocashMessage("CMD_SHOW");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(buffer, 1, 1, commandStream);
            break;
        case CMD_HIDE:
            nocashMessage("CMD_HIDE");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(buffer, 1, 1, commandStream);
            break;
        case CMD_SET_ANIMATION:
            nocashMessage("CMD_SET_ANIMATION");
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(&targetId, 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            nav->set_animation(targetType, targetId, buffer, callingLocation);
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
        case CMD_START_DIALOGUE:
            nocashMessage("CMD_START_DIALOGUE");
            fread(buffer, 2, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            fread(buffer, 4, 1, commandStream);
            fread(buffer, 4, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            fread(&targetType, 1, 1, commandStream);
            if (targetType == TargetType::SPRITE)
                fread(buffer, 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            len = strlen_file(commandStream, 0);
            fread(buffer, len + 1, 1, commandStream);
            break;
        case CMD_WAIT_DIALOGUE_END:
            nocashMessage("CMD_WAIT_DIALOGUE_END");
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
            break;
        case CMD_JUMP_IF_NOT:
            nocashMessage("CMD_JUMP_IF_NOT");
            fread(&address, 4, 1, commandStream);
            break;
        case CMD_JUMP:
            nocashMessage("CMD_JUMP");
            fread(&address, 4, 1, commandStream);
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