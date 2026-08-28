//
// Created by cervi on 27/08/2022.
//

#include "Room/Player.hpp"
#include "ConditionalFile/RoomConditionalFile.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/math.hpp"
#include "Room/Camera.hpp"
#include "Room/Room.hpp"
#include "GameLoop.hpp"
#include <memory>

Player::Player(Room* room) : _room(room) {
  _spr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_spr, "mainchara");

  _upIdleId = _spr->nameToAnimId("upIdle");
  _downIdleId = _spr->nameToAnimId("downIdle");
  _leftIdleId = _spr->nameToAnimId("leftIdle");
  _rightIdleId = _spr->nameToAnimId("rightIdle");
  _upMoveId = _spr->nameToAnimId("upMove");
  _downMoveId = _spr->nameToAnimId("downMove");
  _leftMoveId = _spr->nameToAnimId("leftMove");
  _rightMoveId = _spr->nameToAnimId("rightMove");
  
  Engine::spriteSetShown(_spr, true);
}

void Player::set_player_control(bool playerControl) {
  _control = playerControl;
  if (!_control) {
    if (_spr->_cAnimation == _upMoveId)
      _spr->setAnimation(_upIdleId);
    else if (_spr->_cAnimation == _downMoveId)
      _spr->setAnimation(_downIdleId);
    else if (_spr->_cAnimation == _leftMoveId)
      _spr->setAnimation(_leftIdleId);
    else if (_spr->_cAnimation == _rightMoveId)
      _spr->setAnimation(_rightIdleId);
  }
}

void Player::update() {
  if (!_control)
    return;

  s32 dx = 0, dy = 0;
  int moveDirection = -1;
  bool setAnim = true;
  if (keysHeld() & KEY_DOWN) {
    dy += kMoveSpeed;
    moveDirection = _downMoveId;
    if (_spr->_cAnimation == _downMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_UP) {
    dy -= kMoveSpeed;
    moveDirection = _upMoveId;
    if (_spr->_cAnimation == _upMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_RIGHT) {
    dx += kMoveSpeed;
    moveDirection = _rightMoveId;
    if (_spr->_cAnimation == _rightMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_LEFT) {
    dx -= kMoveSpeed;
    moveDirection = _leftMoveId;
    if (_spr->_cAnimation == _leftMoveId)
      setAnim = false;
  }

  attempt_move(dx, dy);

  if (dx == 0 && dy == 0) {
    if (moveDirection == -1)
      moveDirection = _spr->_cAnimation;
    if (moveDirection == _upMoveId)
      _spr->setAnimation(_upIdleId);
    else if (moveDirection == _downMoveId)
      _spr->setAnimation(_downIdleId);
    else if (moveDirection == _leftMoveId)
      _spr->setAnimation(_leftIdleId);
    else if (moveDirection == _rightMoveId)
      _spr->setAnimation(_rightIdleId);
  } else {
    if (setAnim) {
      _spr->setAnimation(moveDirection);
    }
  }

  if (keysDown() & KEY_A) {
    check_interact();
  }
}

void Player::attempt_move(s32 &dx, s32 &dy) {
  if (dx == 0 && dy == 0)
    return;

  if (!check_collisions(dx, dy)) {
    commit_move(dx, dy);
  }
  else if (!check_collisions(dx, 0)) {
    dy = 0;
    commit_move(dx, dy);
  }
  else if (!check_collisions(0, dy)) {
    dx = 0;
    commit_move(dx, dy);
  }
  
  check_exits();
}

void Player::commit_move(const s32 dx, const s32 dy) {
  _spr->_wx += dx;
  _spr->_wy += dy;

  // Push objects if necessary.
  for (auto &roomSprite : _room->_sprites) {
    roomSprite.second.commit_player_move();
  }
}

void Player::check_exits() {
  u16 width, height;
  _room->_bg.getSize(width, height);
  if (_spr->_wx < 0) {
    _spr->_wx = 0;
    if (_room->_exitLeft != nullptr) {
      scheduleRoom(_room->_exitLeft->_roomId, _room->_exitLeft->_spawnX,
                   _room->_exitLeft->_spawnY);
    }
  } else if ((_spr->_wx >> 8) + 19 > width) {
    _spr->_wx = (width - 19) << 8;
    if (_room->_exitRight != nullptr) {
      scheduleRoom(_room->_exitRight->_roomId, _room->_exitRight->_spawnX,
                   _room->_exitRight->_spawnY);
    }
  }
  if (_spr->_wy < 0) {
    _spr->_wy = 0;
    if (_room->_exitTop != nullptr) {
      scheduleRoom(_room->_exitTop->_roomId, _room->_exitTop->_spawnX,
                   _room->_exitTop->_spawnY);
    }
  } else if ((_spr->_wy >> 8) + 29 > height) {
    _spr->_wy = (height - 29) << 8;
    if (_room->_exitBtm != nullptr) {
      scheduleRoom(_room->_exitBtm->_roomId, _room->_exitBtm->_spawnX,
                   _room->_exitBtm->_spawnY);
    }
  }

  for (auto & collider : _room->_roomData._roomColliders) {
    if (collider._type._type == RoomColliderType::WALL || !collider._enabled)
      continue;
    if (collidesRect(_spr->_wx >> 8, (_spr->_wy >> 8) + 20, 19, 9, collider._x,
                     collider._y, collider._w, collider._h)) {
      switch (collider._type._type) {
      case RoomColliderType::CUTSCENE:
        // Cutscene
        _room->_cutscene = std::make_unique<Cutscene>(
            collider._type._cutscene._cutsceneId, _room->_roomId, _room);
        break;
      case RoomColliderType::EXIT:
        scheduleRoom(collider._type._exit._roomId,
                     collider._type._exit._spawnX,
                     collider._type._exit._spawnY);
        break;
      default:
        break;
      }
    }
  }
}

void Player::check_interact() const {
  s32 x, y, w = 19, h = 9;
  if (_spr->_cAnimation == _upIdleId || _spr->_cAnimation == _upMoveId) {
    x = 0;
    y = -9;
    h = 19;
  } else if (_spr->_cAnimation == _downIdleId ||
             _spr->_cAnimation == _downMoveId) {
    x = 0;
    y = 29;
  } else if (_spr->_cAnimation == _rightIdleId ||
             _spr->_cAnimation == _rightMoveId) {
    x = 19;
    y = 29 - 9;
  } else {
    x = -19;
    y = 29 - 9;
  }
  x += _spr->_wx >> 8;
  y += _spr->_wy >> 8;
  for (auto const &sprite : _room->_sprites) {
    auto &spr = sprite.second;
    if (spr._action != RoomSpriteAction::CUTSCENE)
      continue;
    if (spr._spr->_texture == nullptr)
      continue;

    const s32 x2 = spr._spr->_wx >> 8;
    const s32 y2 = spr._spr->_wy >> 8;
    const s32 w2 = spr._spr->_texture->getWidth();
    const s32 h2 = spr._spr->_texture->getHeight();

    if (collidesRect(x, y, w, h, x2, y2, w2, h2)) {
      if (_room->_cutscene == nullptr)
        _room->_cutscene = std::make_unique<Cutscene>(
            spr._cutscene._cutscene_id, _room->_roomId, _room);
      return;
    }
  }
}

bool Player::check_collisions(s32 dx, s32 dy) const {
  const s32 x = _spr->_wx + dx;
  const s32 y = _spr->_wy + dy;

  for (auto &collider : _room->_roomData._roomColliders) {
    if (!collider._enabled || collider._type._type != RoomColliderType::WALL)
      continue;
    if (collidesRect(x >> 8, (y >> 8) + 20, 19, 9, collider._x, collider._y,
                     collider._w, collider._h)) {
      switch(collider._type._type) {
      case RoomColliderType::WALL:
        return true;
      case RoomColliderType::CUTSCENE:
        return true;
      default:
        break;
      }
    }
  }

  for (auto &roomSprite : _room->_sprites) {
    if (roomSprite.second.check_player_collide(x, y + (20 << 8), 19 << 8,
                                               9 << 8, dx, dy))
      return true;
  }

  return false;
}

void Player::draw() {
  _spr->_cam_x = _room->_camera._pos->_wx;
  _spr->_cam_y = _room->_camera._pos->_wy;
  _spr->_cam_scale_x = _room->_camera._pos->_w_scale_x;
  _spr->_cam_scale_y = _room->_camera._pos->_w_scale_y;
  _spr->_layer = _spr->_wy >> 8;
}

