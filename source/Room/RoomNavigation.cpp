#include "Room/RoomNavigation.hpp"
#include "Cutscene/Navigation.hpp"
#include "Engine/Sprite.hpp"
#include "Room/RoomSprite.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"
#include "Room/Camera.hpp"
#include <memory>
#include <string>

RoomNavigation::RoomNavigation(Room* room) : _room(room) {}

void RoomNavigation::spawn_sprite(const std::string &path, s32 x, s32 y,
                                  s32 layer) {
  _room->_sprites.emplace_back(Engine::Allocated3D, _room).spawn(x, y, path);
}

void RoomNavigation::unload_sprite(s8 sprId) {
  u8 sprId2;
  if (sprId < 0)
    sprId2 = _room->_sprites.size() + sprId;
  else
    sprId2 = sprId;

  if (sprId2 >= _room->_sprites.size())
    return;

  _room->_sprites.erase(_room->_sprites.begin() + sprId2);
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
    u8 targetId2;
    if (targetInfo.targetId < 0)
      targetId2 = _room->_sprites.size() + targetInfo.targetId;
    else
      targetId2 = targetInfo.targetId;

    if (targetId2 >= _room->_sprites.size()) {
      nocashMessage("Error: target id outside of sprite count");
      return nullptr;
    }
    return _room->_sprites[targetId2]._spr;
  }
  default:
    nocashMessage(("Unknown target type for room nav: " +
                   std::to_string(targetInfo.targetType))
                      .c_str());
    return nullptr;
  }
}
