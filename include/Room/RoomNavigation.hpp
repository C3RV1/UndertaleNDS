//
// Created by cervi on 23/06/2026
//

#ifndef UNDERTALE_ROOM_NAVIGATION_HPP
#define UNDERTALE_ROOM_NAVIGATION_HPP

#include "Cutscene/Navigation.hpp"

class Room;

class RoomNavigation : public Navigation {
public:
  explicit RoomNavigation(Room* room);
  void spawn_sprite(u16 sprId, const std::string &path, s32 x, s32 y, s32 layer) final;
  void unload_sprite(u16 sprId) final;
  std::shared_ptr<Engine::Sprite> getTarget(const TargetInfo &targetInfo) final;

private:
  Room* _room;
};

#endif
