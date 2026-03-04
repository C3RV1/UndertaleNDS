//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Navigation.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include <memory>
#include <vector>

void Navigation::spawn_sprite(const std::string &path, s32 x, s32 y, s32 layer,
                              CutsceneLocation callingLocation) {
  if (callingLocation == LOAD_ROOM || callingLocation == ROOM) {
    auto newSprite = std::make_unique<RoomSprite>(Engine::Allocated3D);
    newSprite->spawn(x, y, path);

    globalRoom->_sprites.push_back(std::move(newSprite));
  } else {
    auto newSprite = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
    newSprite->_wx = x;
    newSprite->_wy = y;
    newSprite->_layer = layer;
    Engine::spriteLoadTexture(newSprite, path);
    Engine::spriteSetShown(newSprite, true);

    globalBattle->_sprites.push_back(std::move(newSprite));
  }
}

void Navigation::spawn_relative(const std::string &path,
                                const TargetInfo &targetInfo, s32 dx, s32 dy,
                                s32 layer, CutsceneLocation callingLocation) {
  auto target = getTarget(targetInfo, callingLocation);
  if (target == nullptr)
    return;
  s32 x = target->_wx + dx;
  s32 y = target->_wy + dy;
  spawn_sprite(path, x, y, layer, callingLocation);
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

void Navigation::set_position(const TargetInfo &targetInfo, s32 x, s32 y,
                              CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  if (spriteManager == nullptr)
    return;
  spriteManager->_wx = x;
  spriteManager->_wy = y;
}

void Navigation::move(const TargetInfo &targetInfo, s32 dx, s32 dy,
                      CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  if (spriteManager == nullptr)
    return;
  spriteManager->_wx += dx;
  spriteManager->_wy += dy;
}

void Navigation::set_scale(const TargetInfo &targetInfo, s32 x, s32 y,
                           CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  if (spriteManager == nullptr)
    return;
  spriteManager->_w_scale_x = x;
  spriteManager->_w_scale_y = y;
}

void Navigation::set_shown(const TargetInfo &targetInfo, bool shown,
                           CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  Engine::spriteSetShown(spriteManager, shown);
}

void Navigation::set_animation(const TargetInfo &targetInfo, char *animName,
                               CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  if (spriteManager == nullptr) {
    return;
  }
  int animId = spriteManager->nameToAnimId(animName);
  spriteManager->setAnimation(animId);
}

void Navigation::set_opacity(const TargetInfo &targetInfo, u8 opacity,
                             CutsceneLocation callingLocation) {
  auto spriteManager = getTarget(targetInfo, callingLocation);
  if (spriteManager == nullptr) {
    nocashMessage("no target");
    return;
  }
  spriteManager->_opacity = opacity;
}

void Navigation::set_pos_in_frames(const TargetInfo &targetInfo, s32 x, s32 y,
                                   u16 frames,
                                   CutsceneLocation callingLocation) {
  auto navTask = std::make_unique<NavigationTask>();
  navTask->target = getTarget(targetInfo, callingLocation);
  navTask->startingX = navTask->target->_wx;
  navTask->startingY = navTask->target->_wy;
  navTask->destX = x;
  navTask->destY = y;
  navTask->frames = frames;
  navTask->taskType = NavigationTaskType::POSITION;
  startTask(std::move(navTask));
}

void Navigation::move_in_frames(const TargetInfo &targetInfo, s32 dx, s32 dy,
                                u16 frames, CutsceneLocation callingLocation) {
  auto navTask = std::make_unique<NavigationTask>();
  navTask->target = getTarget(targetInfo, callingLocation);
  navTask->startingX = navTask->target->_wx;
  navTask->startingY = navTask->target->_wy;
  navTask->destX = navTask->target->_wx + dx;
  navTask->destY = navTask->target->_wy + dy;
  navTask->frames = frames;
  navTask->taskType = NavigationTaskType::POSITION;
  startTask(std::move(navTask));
}

void Navigation::scale_in_frames(const TargetInfo &targetInfo, s32 x, s32 y,
                                 u16 frames, CutsceneLocation callingLocation) {
  auto navTask = std::make_unique<NavigationTask>();
  navTask->target = getTarget(targetInfo, callingLocation);
  navTask->startingX = navTask->target->_w_scale_x;
  navTask->startingY = navTask->target->_w_scale_y;
  navTask->destX = x;
  navTask->destY = y;
  navTask->frames = frames;
  navTask->taskType = NavigationTaskType::SCALE;
  startTask(std::move(navTask));
  // nav task not freed as it's managed by navigation
}

void Navigation::startTask(std::unique_ptr<NavigationTask> task) {
  _tasks.push_back(std::move(task));
}

bool Navigation::updateTask(
    std::vector<std::unique_ptr<NavigationTask>>::iterator &taskIter) {
  auto &task = *taskIter;
  auto target = task->target;
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
  if (task->taskType == NavigationTaskType::POSITION) {
    target->_wx = task->startingX + (xRun * task->cFrames) / task->frames;
    target->_wy = task->startingY + (yRun * task->cFrames) / task->frames;
  } else if (task->taskType == NavigationTaskType::SCALE) {
    target->_w_scale_x =
        task->startingX + (xRun * task->cFrames) / task->frames;
    target->_w_scale_y =
        task->startingY + (yRun * task->cFrames) / task->frames;
  }
  return false;
}

void Navigation::endTask(
    std::vector<std::unique_ptr<NavigationTask>>::iterator &taskIter) {
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

void Navigation::clearAllTasks() { _tasks.clear(); }

std::shared_ptr<Engine::Sprite>
Navigation::getTarget(const TargetInfo &targetInfo,
                      CutsceneLocation callingLocation) {
  TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
  if (callingLocation == ROOM || callingLocation == LOAD_ROOM) {
    if (targetType == TargetType::PLAYER) {
      return globalPlayer->_spr;
    } else if (targetType == TargetType::CAMERA) {
      return globalCamera._pos;
    } else if (targetType == TargetType::SPRITE) {
      u8 targetId2;
      if (targetInfo.targetId < 0)
        targetId2 = globalRoom->_sprites.size() + targetInfo.targetId;
      else
        targetId2 = targetInfo.targetId;
      if (targetId2 >= globalRoom->_sprites.size()) {
        nocashMessage("Error: target id outside of sprite count");
        return nullptr;
      }
      return globalRoom->_sprites[targetId2]->_spr;
    }
  } else {
    if (targetType == TargetType::SPRITE) {
      u8 targetId2;
      if (targetInfo.targetId < 0)
        targetId2 = globalBattle->_sprites.size() + targetInfo.targetId;
      else
        targetId2 = targetInfo.targetId;
      if (targetId2 >= globalBattle->_sprites.size()) {
        nocashMessage("Error: target id outside of sprite count");
        return nullptr;
      }
      return globalBattle->_sprites[targetInfo.targetId];
    } else if (targetType == TargetType::ENEMY) {
      u8 enemyTargetId2;
      if (targetInfo.targetId < 0)
        enemyTargetId2 = globalBattle->_enemies.size() + targetInfo.targetId;
      else
        enemyTargetId2 = targetInfo.targetId;
      if (enemyTargetId2 >= globalBattle->_enemies.size()) {
        nocashMessage("Error: target id outside of enemy count");
        return nullptr;
      }
      return globalBattle->_enemies[enemyTargetId2]->getSprite(
          targetInfo.enemySpriteId);
    }
  }
  return nullptr;
}
