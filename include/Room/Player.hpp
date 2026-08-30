//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_PLAYER_HPP
#define UNDERTALE_PLAYER_HPP

#include "Engine/Sprite.hpp"
#include "Room/Camera.hpp"
#include <cstdio>
#include <memory>
#include <nds.h>

class Room;

class Player {
public:
  constexpr static int kMoveSpeed =
      (90 << 8) / 60; // 90 pixels per second, debug speed
  // const int MOVE_SPEED = (70 << 8) / 60;  // 70 pixels per second

  explicit Player(Room* room);
  void update();
  void attempt_move(s32 &dx, s32 &dy);
  void commit_move(s32 dx, s32 dy);
  [[nodiscard]] bool check_collisions(s32 dx, s32 dy) const;
  void check_sprite_interact() const;
  void check_collider_interact() const;
  void set_player_control(bool playerControl);
  void updateDrawPositions(Camera &cam);

  // sprite top left position
  std::shared_ptr<Engine::Sprite> _spr;

private:
  // animation ids
  bool _control = true;
  int _upIdleId;
  int _downIdleId;
  int _leftIdleId;
  int _rightIdleId;
  int _upMoveId;
  int _downMoveId;
  int _leftMoveId;
  int _rightMoveId;

  Room* _room;
};

#endif // UNDERTALE_PLAYER_HPP
