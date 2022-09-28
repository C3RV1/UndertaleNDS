//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_NAVIGATION_HPP
#define UNDERTALE_NAVIGATION_HPP

#include "Sprite.hpp"
#include "CutsceneEnums.hpp"

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
    u16 currentFrames = 0;
    NavigationTaskType taskType = POSITION;
    Engine::Sprite* target = nullptr;
};

class Navigation {
public:
    static void load_texture(char* path,
                             CutsceneLocation callingLocation);
    static void unload_texture(u8 textureId,
                               CutsceneLocation callingLocation);
    static void spawn_sprite(u8 textureId, s32 x, s32 y, s32 layer,
                             CutsceneLocation callingLocation);
    static void unload_sprite(u8 sprId,
                              CutsceneLocation callingLocation);
    static void set_position(u8 targetType, u8 targetId, s32 x, s32 y,
                             CutsceneLocation callingLocation);
    static void set_scale(u8 targetType, u8 targetId, s32 x, s32 y,
                          CutsceneLocation callingLocation);
    static void set_shown(u8 targetType, u8 targetId, bool shown,
                          CutsceneLocation callingLocation);
    static void set_animation(u8 targetType, u8 targetId, char* animName,
                       CutsceneLocation callingLocation);
    void set_pos_in_frames(u8 targetType, u8 targetId, s32 x, s32 y,
                           u16 frames, CutsceneLocation callingLocation);
    void move_in_frames(u8 targetType, u8 targetId, s32 dx, s32 dy,
                        u16 frames, CutsceneLocation callingLocation);
    void scale_in_frames(u8 targetType, u8 targetId, s32 x, s32 y,
                         u16 frames, CutsceneLocation callingLocation);
    void update();
    static Engine::Sprite* getTarget(u8 targetType, u8 targetId,
                                            CutsceneLocation callingLocation);
private:
    void startTask(NavigationTask* navTask);
    bool updateTask(int taskId);
    void endTask(int taskId);
    u8 taskCount = 0;
    NavigationTask** tasks = nullptr;
};

#endif //UNDERTALE_NAVIGATION_HPP
