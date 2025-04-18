//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_NAVIGATION_HPP
#define UNDERTALE_NAVIGATION_HPP

#include "Cutscene/CutsceneEnums.hpp"
#include "Engine/Sprite.hpp"
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
  static void spawn_sprite(const std::string &path, s32 x, s32 y, s32 layer,
                           CutsceneLocation callingLocation);
  static void spawn_relative(const std::string &path,
                             const TargetInfo &targetInfo, s32 dx, s32 dy,
                             s32 layer, CutsceneLocation callingLocation);
  static void unload_sprite(s8 sprId, CutsceneLocation callingLocation);
  static void set_position(const TargetInfo &targetInfo, s32 x, s32 y,
                           CutsceneLocation callingLocation);
  static void move(const TargetInfo &targetInfo, s32 dx, s32 dy,
                   CutsceneLocation callingLocation);
  static void set_scale(const TargetInfo &targetInfo, s32 x, s32 y,
                        CutsceneLocation callingLocation);
  static void set_shown(const TargetInfo &targetInfo, bool shown,
                        CutsceneLocation callingLocation);
  static void set_animation(const TargetInfo &targetInfo, char *animName,
                            CutsceneLocation callingLocation);
  static void set_opacity(const TargetInfo &targetInfo, u8 opacity,
                          CutsceneLocation callingLocation);
  void set_pos_in_frames(const TargetInfo &targetInfo, s32 x, s32 y, u16 frames,
                         CutsceneLocation callingLocation);
  void move_in_frames(const TargetInfo &targetInfo, s32 dx, s32 dy, u16 frames,
                      CutsceneLocation callingLocation);
  void scale_in_frames(const TargetInfo &targetInfo, s32 x, s32 y, u16 frames,
                       CutsceneLocation callingLocation);
  void update();
  void clearAllTasks();
  static std::shared_ptr<Engine::Sprite>
  getTarget(const TargetInfo &targetInfo, CutsceneLocation callingLocation);

private:
  void startTask(std::unique_ptr<NavigationTask> task);
  bool updateTask(std::vector<std::unique_ptr<NavigationTask>>::iterator &task);
  void endTask(std::vector<std::unique_ptr<NavigationTask>>::iterator &task);

  std::vector<std::unique_ptr<NavigationTask>> _tasks;
};

#endif // UNDERTALE_NAVIGATION_HPP
