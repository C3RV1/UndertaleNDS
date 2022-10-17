//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Navigation.hpp"
#include "Battle/Battle.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"

void Navigation::load_texture(char *path, CutsceneLocation callingLocation) {
    auto* newTexture = new Engine::Texture;
    newTexture->loadPath(path);

    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        auto* newTextures = new Engine::Texture*[globalRoom->textureCount + 1];
        memcpy(newTextures, globalRoom->textures, sizeof(Engine::Texture*) * globalRoom->textureCount);

        newTextures[globalRoom->textureCount] = newTexture;
        delete globalRoom->textures;
        globalRoom->textures = newTextures;
        globalRoom->textureCount++;
    } else {
        auto* newTextures = new Engine::Texture*[globalBattle->textureCount + 1];
        memcpy(newTextures, globalBattle->textures, sizeof(Engine::Texture*) * globalBattle->textureCount);

        newTextures[globalBattle->textureCount] = newTexture;
        delete globalBattle->textures;
        globalBattle->textures = newTextures;
        globalBattle->textureCount++;
    }
}

void Navigation::unload_texture(s8 textureId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalRoom->spriteCount + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalRoom->textureCount)
            return;
        auto* sprite = globalRoom->textures[texId2];
        sprite->free_();
        delete sprite;

        auto* newTextures = new Engine::Texture*[globalRoom->textureCount - 1];
        memcpy(newTextures, globalRoom->textures, sizeof(Engine::Texture*) * texId2);
        memcpy(&newTextures[texId2], &globalRoom->textures[texId2 + 1],
               sizeof(Engine::Texture*) * (globalRoom->textureCount - (texId2 + 1)));
        delete globalRoom->textures;
        globalRoom->textures = newTextures;
        globalRoom->textureCount--;
    } else {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalBattle->spriteCount + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalBattle->textureCount)
            return;
        auto* sprite = globalBattle->textures[texId2];
        sprite->free_();
        delete sprite;

        auto* newTextures = new Engine::Texture*[globalBattle->textureCount - 1];
        memcpy(newTextures, globalBattle->textures, sizeof(Engine::Texture*) * texId2);
        memcpy(&newTextures[texId2], &globalBattle->textures[texId2 + 1],
               sizeof(Engine::Texture*) * (globalBattle->textureCount - (texId2 + 1)));
        delete globalBattle->textures;
        globalBattle->textures = newTextures;
        globalBattle->textureCount--;
    }
}

void Navigation::spawn_sprite(s8 textureId, s32 x, s32 y, s32 layer,
                              CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        auto* newSprites = new ManagedSprite*[globalRoom->spriteCount + 1];
        memcpy(newSprites, globalRoom->sprites, sizeof(ManagedSprite*) * globalRoom->spriteCount);

        auto* newRoomSprite = new ManagedSprite(Engine::Allocated3D);
        newRoomSprite->spawn(textureId, x, y, globalRoom->textureCount,
                             globalRoom->textures);

        newSprites[globalRoom->spriteCount] = newRoomSprite;
        delete globalRoom->sprites;
        globalRoom->sprites = newSprites;
        globalRoom->spriteCount++;
    } else {
        auto* newSprites = new ManagedSprite*[globalBattle->spriteCount + 1];
        memcpy(newSprites, globalBattle->sprites, sizeof(ManagedSprite*) * globalBattle->spriteCount);

        auto* newRoomSprite = new ManagedSprite(Engine::AllocatedOAM);
        newRoomSprite->spawn(textureId, x, y,
                             globalBattle->textureCount, globalBattle->textures);
        newRoomSprite->spriteManager.layer = layer;

        newSprites[globalBattle->spriteCount] = newRoomSprite;
        delete globalBattle->sprites;
        globalBattle->sprites = newSprites;
        globalBattle->spriteCount++;
    }
}

void Navigation::spawn_relative(s8 textureId, u8 targetType, s8 targetId, s32 dx, s32 dy, s32 layer,
                                CutsceneLocation callingLocation) {
    Engine::Sprite* target = getTarget(targetType, targetId, callingLocation);
    if (target == nullptr)
        return;
    s32 x = target->wx + dx;
    s32 y = target->wy + dy;
    spawn_sprite(textureId, x, y, layer, callingLocation);
}

void Navigation::unload_sprite(s8 sprId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        u8 sprId2;
        if (sprId < 0)
            sprId2 = globalRoom->spriteCount + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalRoom->spriteCount)
            return;
        auto* sprite = globalRoom->sprites[sprId2];
        sprite->free_();
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalRoom->spriteCount - 1];
        memcpy(newSprites, globalRoom->sprites, sizeof(ManagedSprite*) * sprId2);
        memcpy(&newSprites[sprId2], &globalRoom->sprites[sprId2 + 1],
               sizeof(ManagedSprite*) * (globalRoom->spriteCount - (sprId2 + 1)));
        delete globalRoom->sprites;
        globalRoom->sprites = newSprites;
        globalRoom->spriteCount--;
    } else {
        u8 sprId2;
        if (sprId < 0)
            sprId2 = globalBattle->spriteCount + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalBattle->spriteCount)
            return;
        if (sprId >= globalBattle->spriteCount)
            return;
        auto* sprite = globalBattle->sprites[sprId2];
        sprite->free_();
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalBattle->spriteCount - 1];
        memcpy(newSprites, globalBattle->sprites, sizeof(ManagedSprite*) * sprId2);
        memcpy(&newSprites[sprId2], &globalBattle->sprites[sprId2 + 1],
               sizeof(ManagedSprite*) * (globalBattle->spriteCount - (sprId2 + 1)));
        delete globalBattle->sprites;
        globalBattle->sprites = newSprites;
        globalBattle->spriteCount--;
    }
}

void Navigation::set_position(u8 targetType, s8 targetId, s32 x, s32 y,
                              CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->wx = x;
    spriteManager->wy = y;
}

void Navigation::set_scale(u8 targetType, s8 targetId, s32 x, s32 y,
                           CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->w_scale_x = x;
    spriteManager->w_scale_y = y;
}

void Navigation::set_shown(u8 targetType, s8 targetId, bool shown, CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->setShown(shown);
}

void Navigation::set_animation(u8 targetType, s8 targetId, char *animName, CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr) {
        nocashMessage("no target");
        return;
    }
    if (spriteManager->texture == nullptr) {
        nocashMessage("no sprite");
        return;
    }
    int animId = spriteManager->nameToAnimId(animName);
    spriteManager->setSpriteAnim(animId);
}

void Navigation::set_pos_in_frames(u8 targetType, s8 targetId, s32 x, s32 y, u16 frames,
                                   CutsceneLocation callingLocation) {
    auto* navTask = new NavigationTask;
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->wx;
    navTask->startingY = navTask->target->wy;
    navTask->destX = x;
    navTask->destY = y;
    navTask->frames = frames;
    navTask->taskType = POSITION;
    startTask(navTask);
    // nav task not freed as it's managed by navigation
}

void Navigation::move_in_frames(u8 targetType, s8 targetId, s32 dx, s32 dy, u16 frames,
                                CutsceneLocation callingLocation) {
    auto* navTask = new NavigationTask;
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->wx;
    navTask->startingY = navTask->target->wy;
    navTask->destX = navTask->target->wx + dx;
    navTask->destY = navTask->target->wy + dy;
    navTask->frames = frames;
    navTask->taskType = POSITION;
    startTask(navTask);
    // nav task not freed as it's managed by navigation
}

void Navigation::scale_in_frames(u8 targetType, s8 targetId, s32 x, s32 y, u16 frames,
                                 CutsceneLocation callingLocation) {
    auto* navTask = new NavigationTask;
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->w_scale_x;
    navTask->startingY = navTask->target->w_scale_y;
    navTask->destX = x;
    navTask->destY = y;
    navTask->frames = frames;
    navTask->taskType = SCALE;
    startTask(navTask);
    // nav task not freed as it's managed by navigation
}

void Navigation::startTask(NavigationTask *navTask) {
    auto** newTasks = new NavigationTask*[taskCount + 1];
    memcpy(newTasks, tasks, sizeof(NavigationTask*) * taskCount);
    newTasks[taskCount] = navTask;
    delete[] tasks;
    tasks = newTasks;
    taskCount++;
}

bool Navigation::updateTask(int taskId) {
    if (taskId >= taskCount)
        return false;
    if (tasks == nullptr)
        return false;
    NavigationTask* navTask = tasks[taskId];
    if (navTask == nullptr) {
        endTask(taskId);
        return true;
    }
    Engine::Sprite* target = navTask->target;
    if (navTask->target == nullptr) {
        endTask(taskId);
        return true;
    }
    navTask->currentFrames++;
    if (navTask->currentFrames > navTask->frames) {
        endTask(taskId);
        return true;
    }
    s32 xRun = navTask->destX - navTask->startingX;
    s32 yRun = navTask->destY - navTask->startingY;
    if (navTask->taskType == POSITION) {
        target->wx = navTask->startingX + (xRun * navTask->currentFrames) / navTask->frames;
        target->wy = navTask->startingY + (yRun * navTask->currentFrames) / navTask->frames;
    } else if (navTask->taskType == SCALE) {
        target->w_scale_x = navTask->startingX + (xRun * navTask->currentFrames) / navTask->frames;
        target->w_scale_y = navTask->startingY + (yRun * navTask->currentFrames) / navTask->frames;
    }
    return false;
}

void Navigation::endTask(int taskId) {
    if (taskId >= taskCount)
        return;
    if (tasks == nullptr)
        return;
    auto* task = tasks[taskId];
    delete task;
    tasks[taskId] = nullptr;

    auto** newTasks = new NavigationTask*[taskCount - 1];
    memcpy(newTasks, tasks, sizeof(NavigationTask*) * taskId);
    memcpy(&newTasks[taskId], &tasks[taskId + 1],
           sizeof(NavigationTask*) * (taskCount - (taskId + 1)));
    delete[] tasks;
    tasks = newTasks;
    taskCount--;
}

void Navigation::update() {
    for (int i = 0; i < taskCount; i++) {
        if (updateTask(i)) {  // If we have deleted this entry, next is same id
            i--;
        }
    }
}

void Navigation::clearAllTasks() {
    int taskCountCopy = taskCount;
    for (int i = 0; i < taskCountCopy; i++) {
        endTask(0);
    }
}

Engine::Sprite* Navigation::getTarget(u8 targetType, s8 targetId,
                                      CutsceneLocation callingLocation) {
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        if (targetType == PLAYER) {
            return &globalPlayer->spriteManager;
        } else if (targetType == CAMERA) {
            return &globalCamera.pos;
        } else if (targetType == SPRITE) {
            u8 targetId2;
            if (targetId < 0)
                targetId2 = globalRoom->spriteCount + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalRoom->spriteCount || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalRoom->sprites[targetId2]->spriteManager;
        }
    } else {
        if (targetType == SPRITE) {
            u8 targetId2;
            if (targetId < 0)
                targetId2 = globalBattle->spriteCount + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalBattle->spriteCount || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalBattle->sprites[targetId]->spriteManager;
        }
    }
    return nullptr;
}
