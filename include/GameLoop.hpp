//
// Created by cervi on 25/06/2026
//
#ifndef UNDERTALE_GAME_LOOP_HPP
#define UNDERTALE_GAME_LOOP_HPP

#include "nds.h"
#include "Save.hpp"
#include <memory>

struct ScheduledRoom {
  u16 roomId;
  u16 spawnX, spawnY;
};

void scheduleRoom(u16 roomId, u16 spawnX, u16 spawnY);
void runGameLoop(std::unique_ptr<SaveData> save);

#endif
