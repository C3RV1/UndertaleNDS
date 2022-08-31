//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_NAVIGATION_HPP
#define LAYTON_NAVIGATION_HPP

class Navigation;

#include "SpriteManager.hpp"
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
    int32_t startingX = 0, startingY = 0;
    int32_t destX = 0, destY = 0;
    uint16_t frames = 0;
    uint16_t currentFrames = 0;
    NavigationTaskType taskType = POSITION;
    Engine::SpriteManager* target = nullptr;
};

class Navigation {
public:
    static void spawn_sprite(char* path, int32_t x, int32_t y,
                             CutsceneLocation callingLocation);
    static void unload_sprite(uint8_t sprId,
                              CutsceneLocation callingLocation);
    static void set_position(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                             CutsceneLocation callingLocation);
    static void set_scale(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                          CutsceneLocation callingLocation);
    static void set_shown(uint8_t targetType, uint8_t targetId, bool shown,
                          CutsceneLocation callingLocation);
    static void set_animation(uint8_t targetType, uint8_t targetId, char* animName,
                       CutsceneLocation callingLocation);
    void set_pos_in_frames(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                           uint16_t frames, CutsceneLocation callingLocation);
    void move_in_frames(uint8_t targetType, uint8_t targetId, int32_t dx, int32_t dy,
                        uint16_t frames, CutsceneLocation callingLocation);
    void scale_in_frames(uint8_t targetType, uint8_t targetId, int32_t x, int32_t y,
                         uint16_t frames, CutsceneLocation callingLocation);
    void update();
    static Engine::SpriteManager* getTarget(uint8_t targetType, uint8_t targetId,
                                            CutsceneLocation callingLocation);
private:
    void startTask(NavigationTask* navTask);
    bool updateTask(int taskId);
    void endTask(int taskId);
    uint8_t taskCount = 0;
    NavigationTask** tasks = nullptr;
};

#include "Room.hpp"

#endif //LAYTON_NAVIGATION_HPP
