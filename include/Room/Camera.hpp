//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_CAMERA_HPP
#define UNDERTALE_CAMERA_HPP

#include "Engine/Sprite.hpp"
#include <memory>

class Player;
class Room;

class Camera {
public:
  explicit Camera(Room *room) : _room(room) {
    _pos = std::make_shared<Engine::Sprite>(Engine::NoAlloc);
  }
  
  inline s32 edgeDistanceX() {
    return (128 << 16) / _pos->_w_scale_x;
  }
  
  inline s32 edgeDistanceY() {
    return (96 << 16) / _pos->_w_scale_y;
  }
  
  void updatePosition(Player& player);
  void drawBackground(bool roomChange);
  bool _manual = false;
  int _prevX = 0, _prevY = 0;

  // top left camera position
  // We need it to be a shared_ptr for Navigation.
  std::shared_ptr<Engine::Sprite> _pos;
  Room *_room;
};

#endif // UNDERTALE_CAMERA_HPP
