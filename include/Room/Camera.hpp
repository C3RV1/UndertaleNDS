//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_CAMERA_HPP
#define UNDERTALE_CAMERA_HPP

#include "Engine/Sprite.hpp"
#include <memory>

class Camera {
public:
  Camera() { _pos = std::make_shared<Engine::Sprite>(Engine::NoAlloc); }
  void updatePosition(bool roomChange);
  bool _manual = false;
  int _prevX = 0, _prevY = 0;

  // top left camera position
  // We need it to be a shared_ptr for Navigation.
  std::shared_ptr<Engine::Sprite> _pos;
};

extern Camera globalCamera;

#endif // UNDERTALE_CAMERA_HPP
