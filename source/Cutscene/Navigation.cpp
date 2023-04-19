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
        auto* newTextures = new Engine::Texture*[globalRoom->_textureCount + 1];
        memcpy(newTextures, globalRoom->_textures, sizeof(Engine::Texture*) * globalRoom->_textureCount);

        newTextures[globalRoom->_textureCount] = newTexture;
        delete globalRoom->_textures;
        globalRoom->_textures = newTextures;
        globalRoom->_textureCount++;
    } else {
        auto* newTextures = new Engine::Texture*[globalBattle->_textureCount + 1];
        memcpy(newTextures, globalBattle->_textures, sizeof(Engine::Texture*) * globalBattle->_textureCount);

        newTextures[globalBattle->_textureCount] = newTexture;
        delete globalBattle->_textures;
        globalBattle->_textures = newTextures;
        globalBattle->_textureCount++;
    }
}

void Navigation::unload_texture(s8 textureId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalRoom->_spriteCount + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalRoom->_textureCount)
            return;
        auto* sprite = globalRoom->_textures[texId2];
        delete sprite;

        auto* newTextures = new Engine::Texture*[globalRoom->_textureCount - 1];
        memcpy(newTextures, globalRoom->_textures, sizeof(Engine::Texture*) * texId2);
        memcpy(&newTextures[texId2], &globalRoom->_textures[texId2 + 1],
               sizeof(Engine::Texture*) * (globalRoom->_textureCount - (texId2 + 1)));
        delete globalRoom->_textures;
        globalRoom->_textures = newTextures;
        globalRoom->_textureCount--;
    } else {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalBattle->_spriteCount + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalBattle->_textureCount)
            return;
        auto* sprite = globalBattle->_textures[texId2];
        delete sprite;

        auto* newTextures = new Engine::Texture*[globalBattle->_textureCount - 1];
        memcpy(newTextures, globalBattle->_textures, sizeof(Engine::Texture*) * texId2);
        memcpy(&newTextures[texId2], &globalBattle->_textures[texId2 + 1],
               sizeof(Engine::Texture*) * (globalBattle->_textureCount - (texId2 + 1)));
        delete globalBattle->_textures;
        globalBattle->_textures = newTextures;
        globalBattle->_textureCount--;
    }
}

void Navigation::spawn_sprite(s8 textureId, s32 x, s32 y, s32 layer,
                              CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        auto* newSprites = new ManagedSprite*[globalRoom->_spriteCount + 1];
        memcpy(newSprites, globalRoom->_sprites, sizeof(ManagedSprite*) * globalRoom->_spriteCount);

        auto* newRoomSprite = new ManagedSprite(Engine::Allocated3D);
        newRoomSprite->spawn(textureId, x, y, globalRoom->_textureCount,
                             globalRoom->_textures);

        newSprites[globalRoom->_spriteCount] = newRoomSprite;
        delete globalRoom->_sprites;
        globalRoom->_sprites = newSprites;
        globalRoom->_spriteCount++;
    } else {
        auto* newSprites = new ManagedSprite*[globalBattle->_spriteCount + 1];
        memcpy(newSprites, globalBattle->_sprites, sizeof(ManagedSprite*) * globalBattle->_spriteCount);

        auto* newRoomSprite = new ManagedSprite(Engine::AllocatedOAM);
        newRoomSprite->spawn(textureId, x, y,
                             globalBattle->_textureCount, globalBattle->_textures);
        newRoomSprite->_spr._layer = layer;

        newSprites[globalBattle->_spriteCount] = newRoomSprite;
        delete globalBattle->_sprites;
        globalBattle->_sprites = newSprites;
        globalBattle->_spriteCount++;
    }
}

void Navigation::spawn_relative(s8 textureId, u8 targetType, s8 targetId, s32 dx, s32 dy, s32 layer,
                                CutsceneLocation callingLocation) {
    Engine::Sprite* target = getTarget(targetType, targetId, callingLocation);
    if (target == nullptr)
        return;
    s32 x = target->_wx + dx;
    s32 y = target->_wy + dy;
    spawn_sprite(textureId, x, y, layer, callingLocation);
}

void Navigation::unload_sprite(s8 sprId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        u8 sprId2;
        if (sprId < 0)
            sprId2 = globalRoom->_spriteCount + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalRoom->_spriteCount)
            return;
        auto* sprite = globalRoom->_sprites[sprId2];
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalRoom->_spriteCount - 1];
        memcpy(newSprites, globalRoom->_sprites, sizeof(ManagedSprite*) * sprId2);
        memcpy(&newSprites[sprId2], &globalRoom->_sprites[sprId2 + 1],
               sizeof(ManagedSprite*) * (globalRoom->_spriteCount - (sprId2 + 1)));
        delete globalRoom->_sprites;
        globalRoom->_sprites = newSprites;
        globalRoom->_spriteCount--;
    } else {
        u8 sprId2;
        if (sprId < 0)
            sprId2 = globalBattle->_spriteCount + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalBattle->_spriteCount)
            return;
        if (sprId >= globalBattle->_spriteCount)
            return;
        auto* sprite = globalBattle->_sprites[sprId2];
        delete sprite;

        auto* newSprites = new ManagedSprite*[globalBattle->_spriteCount - 1];
        memcpy(newSprites, globalBattle->_sprites, sizeof(ManagedSprite*) * sprId2);
        memcpy(&newSprites[sprId2], &globalBattle->_sprites[sprId2 + 1],
               sizeof(ManagedSprite*) * (globalBattle->_spriteCount - (sprId2 + 1)));
        delete globalBattle->_sprites;
        globalBattle->_sprites = newSprites;
        globalBattle->_spriteCount--;
    }
}

void Navigation::set_position(u8 targetType, s8 targetId, s32 x, s32 y,
                              CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->_wx = x;
    spriteManager->_wy = y;
}

void Navigation::move(u8 targetType, s8 targetId, s32 dx, s32 dy, CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->_wx += dx;
    spriteManager->_wy += dy;
}

void Navigation::set_scale(u8 targetType, s8 targetId, s32 x, s32 y,
                           CutsceneLocation callingLocation) {
    Engine::Sprite* spriteManager = getTarget(targetType, targetId, callingLocation);
    if (spriteManager == nullptr)
        return;
    spriteManager->_w_scale_x = x;
    spriteManager->_w_scale_y = y;
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
    if (spriteManager->_texture == nullptr) {
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
    navTask->startingX = navTask->target->_wx;
    navTask->startingY = navTask->target->_wy;
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
    navTask->startingX = navTask->target->_wx;
    navTask->startingY = navTask->target->_wy;
    navTask->destX = navTask->target->_wx + dx;
    navTask->destY = navTask->target->_wy + dy;
    navTask->frames = frames;
    navTask->taskType = POSITION;
    startTask(navTask);
    // nav task not freed as it's managed by navigation
}

void Navigation::scale_in_frames(u8 targetType, s8 targetId, s32 x, s32 y, u16 frames,
                                 CutsceneLocation callingLocation) {
    auto* navTask = new NavigationTask;
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->_w_scale_x;
    navTask->startingY = navTask->target->_w_scale_y;
    navTask->destX = x;
    navTask->destY = y;
    navTask->frames = frames;
    navTask->taskType = SCALE;
    startTask(navTask);
    // nav task not freed as it's managed by navigation
}

void Navigation::startTask(NavigationTask *task) {
    auto** newTasks = new NavigationTask*[_taskCount + 1];
    memcpy(newTasks, _tasks, sizeof(NavigationTask*) * _taskCount);
    newTasks[_taskCount] = task;
    delete[] _tasks;
    _tasks = newTasks;
    _taskCount++;
}

bool Navigation::updateTask(int taskId) {
    if (taskId >= _taskCount)
        return false;
    if (_tasks == nullptr)
        return false;
    NavigationTask* navTask = _tasks[taskId];
    if (navTask == nullptr) {
        endTask(taskId);
        return true;
    }
    Engine::Sprite* target = navTask->target;
    if (navTask->target == nullptr) {
        endTask(taskId);
        return true;
    }
    navTask->cFrames++;
    if (navTask->cFrames > navTask->frames) {
        endTask(taskId);
        return true;
    }
    s32 xRun = navTask->destX - navTask->startingX;
    s32 yRun = navTask->destY - navTask->startingY;
    if (navTask->taskType == POSITION) {
        target->_wx = navTask->startingX + (xRun * navTask->cFrames) / navTask->frames;
        target->_wy = navTask->startingY + (yRun * navTask->cFrames) / navTask->frames;
    } else if (navTask->taskType == SCALE) {
        target->_w_scale_x = navTask->startingX + (xRun * navTask->cFrames) / navTask->frames;
        target->_w_scale_y = navTask->startingY + (yRun * navTask->cFrames) / navTask->frames;
    }
    return false;
}

void Navigation::endTask(int taskId) {
    if (taskId >= _taskCount)
        return;
    if (_tasks == nullptr)
        return;
    auto* task = _tasks[taskId];
    delete task;
    _tasks[taskId] = nullptr;

    auto** newTasks = new NavigationTask*[_taskCount - 1];
    memcpy(newTasks, _tasks, sizeof(NavigationTask*) * taskId);
    memcpy(&newTasks[taskId], &_tasks[taskId + 1],
           sizeof(NavigationTask*) * (_taskCount - (taskId + 1)));
    delete[] _tasks;
    _tasks = newTasks;
    _taskCount--;
}

void Navigation::update() {
    for (int i = 0; i < _taskCount; i++) {
        if (updateTask(i)) {  // If we have deleted this entry, next is same id
            i--;
        }
    }
}

void Navigation::clearAllTasks() {
    int taskCountCopy = _taskCount;
    for (int i = 0; i < taskCountCopy; i++) {
        endTask(0);
    }
}

Engine::Sprite* Navigation::getTarget(u8 targetType, s8 targetId,
                                      CutsceneLocation callingLocation) {
    if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
        if (targetType == PLAYER) {
            return &globalPlayer->_playerSpr;
        } else if (targetType == CAMERA) {
            return &globalCamera._pos;
        } else if (targetType == SPRITE) {
            u8 targetId2;
            if (targetId < 0)
                targetId2 = globalRoom->_spriteCount + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalRoom->_spriteCount || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalRoom->_sprites[targetId2]->_spr;
        }
    } else {
        if (targetType == SPRITE) {
            u8 targetId2;
            if (targetId < 0)
                targetId2 = globalBattle->_spriteCount + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalBattle->_spriteCount || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalBattle->_sprites[targetId]->_spr;
        }
    }
    return nullptr;
}
