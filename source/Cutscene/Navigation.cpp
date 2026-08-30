//
// Created by cervi on 30/08/2022.
//

#include "Cutscene/Navigation.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/math.hpp"
#include <memory>
#include <string>
#include <vector>

TargetInfo readTarget(BufferReader &br) {
  TargetInfo targetInfo;
  br.read(&targetInfo.targetType, 1);
  TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
  if (targetType == TargetType::SPRITE)
    br.read(&targetInfo.targetId, 2);
  else if (targetType == TargetType::ENEMY) {
    br.read(&targetInfo.enemyTargetId, 1);
    br.read(&targetInfo.enemySpriteId, 1);
  }
  return targetInfo;
}

void Navigation::spawn_relative(u16 sprId, const std::string &path,
                                const TargetInfo &targetInfo, s32 dx, s32 dy,
                                s32 layer) {
  auto target = getTarget(targetInfo);
  if (target == nullptr)
    return;
  s32 x = target->_wx + dx;
  s32 y = target->_wy + dy;
  spawn_sprite(sprId, path, x, y, layer);
}

void Navigation::set_position(const TargetInfo &targetInfo, s32 x, s32 y) {
  auto spriteManager = getTarget(targetInfo);
  if (spriteManager == nullptr)
    return;
  spriteManager->_wx = x;
  spriteManager->_wy = y;
}

void Navigation::move(const TargetInfo &targetInfo, s32 dx, s32 dy) {
  auto spriteManager = getTarget(targetInfo);
  if (spriteManager == nullptr)
    return;
  spriteManager->_wx += dx;
  spriteManager->_wy += dy;
}

void Navigation::set_scale(const TargetInfo &targetInfo, s32 x, s32 y) {
  auto spriteManager = getTarget(targetInfo);
  if (spriteManager == nullptr)
    return;
  spriteManager->_w_scale_x = x;
  spriteManager->_w_scale_y = y;
}

void Navigation::set_shown(const TargetInfo &targetInfo, bool shown) {
  auto spriteManager = getTarget(targetInfo);
  Engine::spriteSetShown(spriteManager, shown);
}

void Navigation::set_animation(const TargetInfo &targetInfo,
                               const std::string &animName) {
  auto spriteManager = getTarget(targetInfo);
  if (spriteManager == nullptr)
    return;
  int animId = spriteManager->nameToAnimId(animName);
  spriteManager->setAnimation(animId);
}

void Navigation::set_opacity(const TargetInfo &targetInfo, u8 opacity) {
  auto spriteManager = getTarget(targetInfo);
  if (spriteManager == nullptr)
    return;
  spriteManager->_opacity = opacity;
}

void Navigation::set_pos_in_frames(const TargetInfo &targetInfo, s32 x, s32 y,
                                   u16 frames) {
  NavigationTask navTask;
  navTask.target = getTarget(targetInfo);
  navTask.startingX = navTask.target->_wx;
  navTask.startingY = navTask.target->_wy;
  navTask.destX = x;
  navTask.destY = y;
  navTask.frames = frames;
  navTask.taskType = NavigationTaskType::POSITION;
  startTask(std::move(navTask));
}

void Navigation::move_in_frames(const TargetInfo &targetInfo, s32 dx, s32 dy,
                                u16 frames) {
  NavigationTask navTask;
  navTask.target = getTarget(targetInfo);
  navTask.startingX = navTask.target->_wx;
  navTask.startingY = navTask.target->_wy;
  navTask.destX = navTask.target->_wx + dx;
  navTask.destY = navTask.target->_wy + dy;
  navTask.frames = frames;
  navTask.taskType = NavigationTaskType::POSITION;
  startTask(std::move(navTask));
}

void Navigation::scale_in_frames(const TargetInfo &targetInfo, s32 x, s32 y,
                                 u16 frames) {
  NavigationTask navTask;
  navTask.target = getTarget(targetInfo);
  navTask.startingX = navTask.target->_w_scale_x;
  navTask.startingY = navTask.target->_w_scale_y;
  navTask.destX = x;
  navTask.destY = y;
  navTask.frames = frames;
  navTask.taskType = NavigationTaskType::SCALE;
  startTask(std::move(navTask));
}

void Navigation::startTask(NavigationTask task) {
  _tasks.push_back(std::move(task));
}

bool Navigation::updateTask(
    std::vector<NavigationTask>::iterator &taskIter) {
  auto &task = *taskIter;
  auto target = task.target;
  if (task.target == nullptr) {
    endTask(taskIter);
    return true;
  }
  task.cFrames++;
  if (task.cFrames > task.frames) {
    endTask(taskIter);
    return true;
  }
  
  if (task.taskType == NavigationTaskType::POSITION) {
    target->_wx = lerp(task.startingX, task.destX, task.cFrames, task.frames);
    target->_wy = lerp(task.startingY, task.destY, task.cFrames, task.frames);
  } else if (task.taskType == NavigationTaskType::SCALE) {
    target->_w_scale_x =
        lerp(task.startingX, task.destX, task.cFrames, task.frames);
    target->_w_scale_y =
        lerp(task.startingY, task.destY, task.cFrames, task.frames);
  }
  return false;
}

void Navigation::endTask(std::vector<NavigationTask>::iterator &taskIter) {
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

