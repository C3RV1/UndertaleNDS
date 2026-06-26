//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_NAVIGATION_HPP
#define UNDERTALE_NAVIGATION_HPP

#include "Engine/Sprite.hpp"
#include "Formats/utils.hpp"
#include <memory>
#include <vector>

enum class NavigationTaskType { POSITION = 0, SCALE = 1 };

enum class TargetType {
  NULL_ = 0,
  PLAYER = 1,
  SPRITE = 2,
  CAMERA = 3,
  ENEMY = 4
};

struct TargetInfo {
  u8 targetType;

  // If targetType == SPRITE
  s8 targetId;

  // If targetType == ENEMY
  s8 enemySpriteId;
};

TargetInfo readTarget(BufferReader &br);

struct NavigationTask {
  s32 startingX = 0, startingY = 0;
  s32 destX = 0, destY = 0;
  u16 frames = 0;
  u16 cFrames = 0;
  NavigationTaskType taskType = NavigationTaskType::POSITION;
  std::shared_ptr<Engine::Sprite> target = nullptr;
};

class Navigation {
public:
  virtual void spawn_sprite(const std::string &path, s32 x, s32 y, s32 layer) = 0;
  void spawn_relative(const std::string &path, const TargetInfo &targetInfo,
                      s32 dx, s32 dy, s32 layer);
  virtual void unload_sprite(s8 sprId) = 0;
  void set_position(const TargetInfo &targetInfo, s32 x, s32 y);
  void move(const TargetInfo &targetInfo, s32 dx, s32 dy);
  void set_scale(const TargetInfo &targetInfo, s32 x, s32 y);
  void set_shown(const TargetInfo &targetInfo, bool shown);
  void set_animation(const TargetInfo &targetInfo, const std::string& animName);
  void set_opacity(const TargetInfo &targetInfo, u8 opacity);
  void set_pos_in_frames(const TargetInfo &targetInfo, s32 x, s32 y,
                         u16 frames);
  void move_in_frames(const TargetInfo &targetInfo, s32 dx, s32 dy, u16 frames);
  void scale_in_frames(const TargetInfo &targetInfo, s32 x, s32 y, u16 frames);
  void update();
  void clearAllTasks();
  virtual std::shared_ptr<Engine::Sprite>
  getTarget(const TargetInfo &targetInfo) = 0;

private:
  void startTask(NavigationTask task);
  bool updateTask(std::vector<NavigationTask>::iterator &task);
  void endTask(std::vector<NavigationTask>::iterator &task);

  std::vector<NavigationTask> _tasks;
};


#endif // UNDERTALE_NAVIGATION_HPP
