//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Navigation.hpp"

void Navigation::spawn_sprite(char *path, int32_t x, int32_t y,
                              CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        auto* newSprites = new ManagedSprite*[globalRoom->spriteCount + 1];
        memcpy(newSprites, globalRoom->sprites, sizeof(ManagedSprite*) * globalRoom->spriteCount);

        auto* newRoomSprite = new ManagedSprite;
        newRoomSprite->spawn(path, x, y);

        newSprites[globalRoom->spriteCount] = newRoomSprite;
        delete globalRoom->sprites;
        globalRoom->sprites = newSprites;
        globalRoom->spriteCount++;
    } else {
        auto* newSprites = new ManagedSprite*[globalBattle->spriteCount + 1];
        memcpy(newSprites, globalBattle->sprites, sizeof(ManagedSprite*) * globalBattle->spriteCount);

        auto* newRoomSprite = new ManagedSprite;
        newRoomSprite->spawn(path, x, y);

        newSprites[globalBattle->spriteCount] = newRoomSprite;
        delete globalBattle->sprites;
        globalBattle->sprites = newSprites;
        globalBattle->spriteCount++;
    }
}

void Navigation::unload_sprite(uint8_t sprId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        if (sprId >= globalRoom->spriteCount)
            return;
        auto* sprite = globalRoom->sprites[sprId];
        sprite->free_();
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalRoom->spriteCount - 1];
        memcpy(newSprites, globalRoom->sprites, sizeof(ManagedSprite*) * sprId);
        memcpy(&newSprites[sprId], &globalRoom->sprites[sprId + 1],
               sizeof(ManagedSprite*) * (globalRoom->spriteCount - (sprId + 1)));
        delete globalRoom->sprites;
        globalRoom->sprites = newSprites;
        globalRoom->spriteCount--;
    } else {
        if (sprId >= globalBattle->spriteCount)
            return;
        auto* sprite = globalBattle->sprites[sprId];
        sprite->free_();
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalBattle->spriteCount - 1];
        memcpy(newSprites, globalBattle->sprites, sizeof(ManagedSprite*) * sprId);
        memcpy(&newSprites[sprId], &globalBattle->sprites[sprId + 1],
               sizeof(ManagedSprite*) * (globalBattle->spriteCount - (sprId + 1)));
        delete globalBattle->sprites;
        globalBattle->sprites = newSprites;
        globalBattle->spriteCount--;
    }
}

void Navigation::set_position(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                              CutsceneLocation callingLocation) {
    Engine::SpriteManager* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->wx = x;
    spriteManager->wy = y;
}

void Navigation::set_scale(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                           CutsceneLocation callingLocation) {
    Engine::SpriteManager* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->wscale_x = x;
    spriteManager->wscale_y = y;
}

void Navigation::set_shown(uint8_t targetType, uint8_t targetId, bool shown, CutsceneLocation callingLocation) {
    Engine::SpriteManager* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->setShown(shown);
}

void Navigation::set_animation(uint8_t targetType, uint8_t targetId, char *animName, CutsceneLocation callingLocation) {
    Engine::SpriteManager* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr) {
        nocashMessage("no target");
        return;
    }
    if (spriteManager->sprite == nullptr) {
        nocashMessage("no sprite");
        return;
    }
    int animId = spriteManager->nameToAnimId(animName);
    spriteManager->setSpriteAnim(animId);
}

void Navigation::set_pos_in_frames(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y, uint16_t frames,
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

void Navigation::move_in_frames(uint8_t targetType, uint8_t targetId, int32_t dx, int32_t dy, uint16_t frames,
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

void Navigation::scale_in_frames(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y, uint16_t frames,
                                 CutsceneLocation callingLocation) {
    auto* navTask = new NavigationTask;
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->wscale_x;
    navTask->startingY = navTask->target->wscale_y;
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
    NavigationTask* navTask = tasks[taskId];
    if (navTask == nullptr) {
        endTask(taskId);
        return true;
    }
    Engine::SpriteManager* target = navTask->target;
    if (navTask->target == nullptr) {
        endTask(taskId);
        return true;
    }
    if (navTask->currentFrames > navTask->frames) {
        endTask(taskId);
        return true;
    }
    int32_t xRun = navTask->destX - navTask->startingX;
    int32_t yRun = navTask->destY - navTask->startingY;
    if (navTask->taskType == POSITION) {
        target->wx = navTask->startingX + (xRun * navTask->currentFrames) / navTask->frames;
        target->wy = navTask->startingY + (yRun * navTask->currentFrames) / navTask->frames;
    } else if (navTask->taskType == SCALE) {
        target->wscale_x = navTask->startingX + (xRun * navTask->currentFrames) / navTask->frames;
        target->wscale_y = navTask->startingY + (yRun * navTask->currentFrames) / navTask->frames;
    }
    navTask->currentFrames++;
    return false;
}

void Navigation::endTask(int taskId) {
    if (taskId >= taskCount)
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

Engine::SpriteManager* Navigation::getTarget(uint8_t targetType, uint8_t targetId,
                                             CutsceneLocation callingLocation) {
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        if (targetType == PLAYER) {
            nocashMessage("player");
            return &globalPlayer->spriteManager;
        } else if (targetType == CAMERA) {
            return &globalCamera.pos;
        } else if (targetType == SPRITE) {
            if (targetId >= globalRoom->spriteCount) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            } else {
                return &globalRoom->sprites[targetId]->spriteManager;
            }
        }
    } else {
        if (targetType == SPRITE) {
            if (targetId >= globalBattle->spriteCount) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            } else {
                return &globalBattle->sprites[targetId]->spriteManager;
            }
        }
    }
    return nullptr;
}
