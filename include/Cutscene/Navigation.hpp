//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_NAVIGATION_HPP
#define UNDERTALE_NAVIGATION_HPP

#include "Engine/Sprite.hpp"
#include "Cutscene/CutsceneEnums.hpp"

enum NavigationTaskType {
    POSITION = 0,
    SCALE = 1
};

enum TargetType {
    NULL_ = 0,
    PLAYER = 1,
    SPRITE = 2,
    CAMERA = 3
};

struct NavigationTask {
    s32 startingX = 0, startingY = 0;
    s32 destX = 0, destY = 0;
    u16 frames = 0;
    u16 cFrames = 0;
    NavigationTaskType taskType = POSITION;
    Engine::Sprite* target = nullptr;
};

class Navigation {
public:
    static void load_texture(const std::string& path,
                             CutsceneLocation callingLocation);
    static void unload_texture(s8 textureId,
                               CutsceneLocation callingLocation);
    static void spawn_sprite(s8 textureId, s32 x, s32 y, s32 layer,
                             CutsceneLocation callingLocation);
    static void spawn_relative(s8 textureId,
                               u8 targetType, s8 targetId, s32 dx, s32 dy, s32 layer,
                               CutsceneLocation callingLocation);
    static void unload_sprite(s8 sprId,
                              CutsceneLocation callingLocation);
    static void set_position(u8 targetType, s8 targetId, s32 x, s32 y,
                             CutsceneLocation callingLocation);
    static void move(u8 targetType, s8 targetId, s32 dx, s32 dy,
                     CutsceneLocation callingLocation);
    static void set_scale(u8 targetType, s8 targetId, s32 x, s32 y,
                          CutsceneLocation callingLocation);
    static void set_shown(u8 targetType, s8 targetId, bool shown,
                          CutsceneLocation callingLocation);
    static void set_animation(u8 targetType, s8 targetId, char* animName,
                       CutsceneLocation callingLocation);
    void set_pos_in_frames(u8 targetType, s8 targetId, s32 x, s32 y,
                           u16 frames, CutsceneLocation callingLocation);
    void move_in_frames(u8 targetType, s8 targetId, s32 dx, s32 dy,
                        u16 frames, CutsceneLocation callingLocation);
    void scale_in_frames(u8 targetType, s8 targetId, s32 x, s32 y,
                         u16 frames, CutsceneLocation callingLocation);
    void update();
    void clearAllTasks();
    static Engine::Sprite* getTarget(u8 targetType, s8 targetId,
                                     CutsceneLocation callingLocation);
private:
    void startTask(std::unique_ptr<NavigationTask> task);
    bool updateTask(std::vector<std::unique_ptr<NavigationTask>>::iterator& task);
    void endTask(std::vector<std::unique_ptr<NavigationTask>>::iterator& task);
    std::vector<std::unique_ptr<NavigationTask>> _tasks;
};

#endif //UNDERTALE_NAVIGATION_HPP
