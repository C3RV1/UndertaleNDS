//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Navigation.hpp"
#include "Battle/Battle.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"
#include <algorithm>

void Navigation::load_texture(const std::string& path, CutsceneLocation callingLocation) {
    auto newTexture = std::make_shared<Engine::Texture>();
    newTexture->loadPath(path);

    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        globalRoom->_textures.push_back(newTexture);
    } else {
        globalBattle->_textures.push_back(newTexture);
    }
}

void Navigation::unload_texture(s8 textureId, CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalRoom->_textures.size() + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalRoom->_textures.size())
            return;
        globalRoom->_textures.erase(globalRoom->_textures.begin() + texId2);
    } else {
        u8 texId2;
        if (textureId < 0)
            texId2 = globalBattle->_textures.size() + textureId;
        else
            texId2 = textureId;
        if (texId2 >= globalBattle->_textures.size())
            return;
        globalBattle->_textures.erase(globalBattle->_textures.begin() + texId2);
    }
}

void Navigation::spawn_sprite(s8 textureId, s32 x, s32 y, s32 layer,
                              CutsceneLocation callingLocation) {
    if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
        auto newSprite = std::make_unique<ManagedSprite>(Engine::Allocated3D);
        newSprite->spawn(textureId, x, y, globalRoom->_textures);
        globalRoom->_sprites.push_back(std::move(newSprite));
    } else {
        auto newSprite = std::make_unique<ManagedSprite>(Engine::AllocatedOAM);
        newSprite->spawn(textureId, x, y, globalBattle->_textures);
        newSprite->_spr._layer = layer;
        globalBattle->_sprites.push_back(std::move(newSprite));
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
            sprId2 = globalRoom->_sprites.size() + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalRoom->_sprites.size())
            return;
        globalRoom->_sprites.erase(globalRoom->_sprites.begin() + sprId2);
    } else {
        u8 sprId2;
        if (sprId < 0)
            sprId2 = globalBattle->_sprites.size() + sprId;
        else
            sprId2 = sprId;
        if (sprId2 >= globalBattle->_sprites.size())
            return;
        globalBattle->_sprites.erase(globalBattle->_sprites.begin() + sprId2);
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
        nocashMessage("no texture");
        return;
    }
    int animId = spriteManager->nameToAnimId(animName);
    spriteManager->setAnimation(animId);
}

void Navigation::set_pos_in_frames(u8 targetType, s8 targetId, s32 x, s32 y, u16 frames,
                                   CutsceneLocation callingLocation) {
    auto navTask = std::make_unique<NavigationTask>();
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->_wx;
    navTask->startingY = navTask->target->_wy;
    navTask->destX = x;
    navTask->destY = y;
    navTask->frames = frames;
    navTask->taskType = POSITION;
    startTask(std::move(navTask));
}

void Navigation::move_in_frames(u8 targetType, s8 targetId, s32 dx, s32 dy, u16 frames,
                                CutsceneLocation callingLocation) {
    auto navTask = std::make_unique<NavigationTask>();
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->_wx;
    navTask->startingY = navTask->target->_wy;
    navTask->destX = navTask->target->_wx + dx;
    navTask->destY = navTask->target->_wy + dy;
    navTask->frames = frames;
    navTask->taskType = POSITION;
    startTask(std::move(navTask));
}

void Navigation::scale_in_frames(u8 targetType, s8 targetId, s32 x, s32 y, u16 frames,
                                 CutsceneLocation callingLocation) {
    auto navTask = std::make_unique<NavigationTask>();
    navTask->target = getTarget(targetType, targetId, callingLocation);
    navTask->startingX = navTask->target->_w_scale_x;
    navTask->startingY = navTask->target->_w_scale_y;
    navTask->destX = x;
    navTask->destY = y;
    navTask->frames = frames;
    navTask->taskType = SCALE;
    startTask(std::move(navTask));
    // nav task not freed as it's managed by navigation
}

void Navigation::startTask(std::unique_ptr<NavigationTask> task) {
    _tasks.push_back(std::move(task));
}

bool Navigation::updateTask(std::vector<std::unique_ptr<NavigationTask>>::iterator& taskIter) {
    auto & task = *taskIter;
    Engine::Sprite* target = task->target;
    if (task->target == nullptr) {
        endTask(taskIter);
        return true;
    }
    task->cFrames++;
    if (task->cFrames > task->frames) {
        endTask(taskIter);
        return true;
    }
    s32 xRun = task->destX - task->startingX;
    s32 yRun = task->destY - task->startingY;
    if (task->taskType == POSITION) {
        target->_wx = task->startingX + (xRun * task->cFrames) / task->frames;
        target->_wy = task->startingY + (yRun * task->cFrames) / task->frames;
    } else if (task->taskType == SCALE) {
        target->_w_scale_x = task->startingX + (xRun * task->cFrames) / task->frames;
        target->_w_scale_y = task->startingY + (yRun * task->cFrames) / task->frames;
    }
    return false;
}

void Navigation::endTask(std::vector<std::unique_ptr<NavigationTask>>::iterator& taskIter) {
    taskIter = _tasks.erase(taskIter);
}

void Navigation::update() {
    for (auto taskIter = _tasks.begin(); taskIter != _tasks.end();) {
        if (updateTask(taskIter)) {
            // If we have deleted this task, taskIter is already updated
            continue;
        }
        taskIter++;
    }
}

void Navigation::clearAllTasks() {
    _tasks.clear();
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
                targetId2 = globalRoom->_sprites.size() + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalRoom->_sprites.size() || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalRoom->_sprites[targetId2]->_spr;
        }
    } else {
        if (targetType == SPRITE) {
            u8 targetId2;
            if (targetId < 0)
                targetId2 = globalBattle->_sprites.size() + targetId;
            else
                targetId2 = targetId;
            if (targetId2 >= globalBattle->_sprites.size() || targetId2 < 0) {
                nocashMessage("Error: target id outside of sprite count");
                return nullptr;
            }
            return &globalBattle->_sprites[targetId]->_spr;
        }
    }
    return nullptr;
}
