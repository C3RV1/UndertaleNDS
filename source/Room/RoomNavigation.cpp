#include "Room/RoomNavigation.hpp"
#include "Cutscene/Navigation.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Room/RoomSprite.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"
#include <memory>
#include <string>
#include <tuple>
#include <utility>

RoomNavigation::RoomNavigation(Room* room) : _room(room) {}

void RoomNavigation::spawn_sprite(u16 sprId, const std::string &path, s32 x,
                                  s32 y, s32 layer) {
  if (sprId != 0 && _room->_sprites.count(sprId) > 0)
    Engine::throw_("RoomNav: Duplicate spr id != 0: " + std::to_string(sprId));
  _room->_sprites
      .emplace(std::piecewise_construct, std::make_tuple(sprId),
               std::make_tuple(Engine::Allocated3D, _room))
      ->second.spawn(x, y, path);
}

void RoomNavigation::unload_sprite(u16 sprId) {
  if (sprId == 0)
    return;
  _room->_sprites.erase(sprId);
}

std::shared_ptr<Engine::Sprite>
RoomNavigation::getTarget(const TargetInfo &targetInfo) {
  TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
  switch (targetType) {
  case TargetType::PLAYER:
    return _room->_player._spr;
  case TargetType::CAMERA:
    return _room->_camera._pos;
  case TargetType::SPRITE: {
    auto it = _room->_sprites.find(targetInfo.targetId);
    if (it != _room->_sprites.end())
      return it->second._spr;
    Engine::log_("RoomNav: Target sprite with id " +
                 std::to_string(targetInfo.targetId) + " not found.");
    return nullptr;
  }
  default:
    Engine::log_("Unknown target type for room nav: " +
                 std::to_string(targetInfo.targetType));
    return nullptr;
  }
}
