//
// Created by cervi on 27/08/2022.
//

#include "Room/Player.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/math.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Room/Camera.hpp"
#include "Room/Room.hpp"
#include <memory>

Player::Player() {
  _playerSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_playerSpr, "mainchara");

  _upIdleId = _playerSpr->nameToAnimId("upIdle");
  _downIdleId = _playerSpr->nameToAnimId("downIdle");
  _leftIdleId = _playerSpr->nameToAnimId("leftIdle");
  _rightIdleId = _playerSpr->nameToAnimId("rightIdle");
  _upMoveId = _playerSpr->nameToAnimId("upMove");
  _downMoveId = _playerSpr->nameToAnimId("downMove");
  _leftMoveId = _playerSpr->nameToAnimId("leftMove");
  _rightMoveId = _playerSpr->nameToAnimId("rightMove");
}

void Player::set_player_control(bool playerControl) {
  _playerControl = playerControl;
  if (!_playerControl) {
    if (_playerSpr->_cAnimation == _upMoveId)
      _playerSpr->setAnimation(_upIdleId);
    else if (_playerSpr->_cAnimation == _downMoveId)
      _playerSpr->setAnimation(_downIdleId);
    else if (_playerSpr->_cAnimation == _leftMoveId)
      _playerSpr->setAnimation(_leftIdleId);
    else if (_playerSpr->_cAnimation == _rightMoveId)
      _playerSpr->setAnimation(_rightIdleId);
  }
}

void Player::update() {
  if (!_playerControl)
    return;

  s32 dx = 0, dy = 0;
  int moveDirection = -1;
  bool setAnim = true;
  if (keysHeld() & KEY_DOWN) {
    dy += kMoveSpeed;
    moveDirection = _downMoveId;
    if (_playerSpr->_cAnimation == _downMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_UP) {
    dy -= kMoveSpeed;
    moveDirection = _upMoveId;
    if (_playerSpr->_cAnimation == _upMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_RIGHT) {
    dx += kMoveSpeed;
    moveDirection = _rightMoveId;
    if (_playerSpr->_cAnimation == _rightMoveId)
      setAnim = false;
  }
  if (keysHeld() & KEY_LEFT) {
    dx -= kMoveSpeed;
    moveDirection = _leftMoveId;
    if (_playerSpr->_cAnimation == _leftMoveId)
      setAnim = false;
  }

  attempt_move(dx, dy);

  if (dx == 0 && dy == 0) {
    if (moveDirection == -1)
      moveDirection = _playerSpr->_cAnimation;
    if (moveDirection == _upMoveId)
      _playerSpr->setAnimation(_upIdleId);
    else if (moveDirection == _downMoveId)
      _playerSpr->setAnimation(_downIdleId);
    else if (moveDirection == _leftMoveId)
      _playerSpr->setAnimation(_leftIdleId);
    else if (moveDirection == _rightMoveId)
      _playerSpr->setAnimation(_rightIdleId);
  } else {
    if (setAnim) {
      _playerSpr->setAnimation(moveDirection);
    }
  }

  if (keysDown() & KEY_A) {
    check_interact();
  }
}

void Player::attempt_move(s32 &dx, s32 &dy) {
  if (dx == 0 && dy == 0)
    return;

  check_exits();

  if (!check_collisions(dx, dy)) {
    commit_move(dx, dy);
    return;
  }

  if (!check_collisions(dx, 0)) {
    dy = 0;
    commit_move(dx, dy);
    return;
  }

  if (!check_collisions(0, dy)) {
    dx = 0;
    commit_move(dx, dy);
    return;
  }
}

void Player::commit_move(const s32 dx, const s32 dy) {
  _playerSpr->_wx += dx;
  _playerSpr->_wy += dy;

  // Push objects if necessary.
  for (auto &roomSprite : globalRoom->_sprites) {
    roomSprite->commit_player_move();
  }
}

void Player::check_exits() {
  u16 width, height;
  globalRoom->_bg.getSize(width, height);
  if (_playerSpr->_wx < 0) {
    _playerSpr->_wx = 0;
    if (globalRoom->_exitLeft != nullptr) {
      loadNewRoom(globalRoom->_exitLeft->roomId, globalRoom->_exitLeft->spawnX,
                  globalRoom->_exitLeft->spawnY);
    }
  } else if ((_playerSpr->_wx >> 8) + 19 > width) {
    _playerSpr->_wx = (width - 19) << 8;
    if (globalRoom->_exitRight != nullptr) {
      loadNewRoom(globalRoom->_exitRight->roomId,
                  globalRoom->_exitRight->spawnX,
                  globalRoom->_exitRight->spawnY);
    }
  }
  if (_playerSpr->_wy < 0) {
    _playerSpr->_wy = 0;
    if (globalRoom->_exitTop != nullptr) {
      loadNewRoom(globalRoom->_exitTop->roomId, globalRoom->_exitTop->spawnX,
                  globalRoom->_exitTop->spawnY);
    }
  } else if ((_playerSpr->_wy >> 8) + 29 > height) {
    _playerSpr->_wy = (height - 29) << 8;
    if (globalRoom->_exitBtm != nullptr) {
      loadNewRoom(globalRoom->_exitBtm->roomId, globalRoom->_exitBtm->spawnX,
                  globalRoom->_exitBtm->spawnY);
    }
  }

  for (int i = 0; i < globalRoom->_rectExitCount; i++) {
    ROOMExit *rectExit = globalRoom->_rectExits[i];
    if (collidesRect(_playerSpr->_wx >> 8, (_playerSpr->_wy >> 8) + 20, 19, 9,
                     rectExit->x, rectExit->y, rectExit->w, rectExit->h)) {
      loadNewRoom(rectExit->roomId, rectExit->spawnX, rectExit->spawnY);
    }
  }
}

void Player::check_interact() const {
  s32 x, y, w = 19, h = 9;
  if (_playerSpr->_cAnimation == _upIdleId ||
      _playerSpr->_cAnimation == _upMoveId) {
    x = 0;
    y = -9;
    h = 19;
  } else if (_playerSpr->_cAnimation == _downIdleId ||
             _playerSpr->_cAnimation == _downMoveId) {
    x = 0;
    y = 29;
  } else if (_playerSpr->_cAnimation == _rightIdleId ||
             _playerSpr->_cAnimation == _rightMoveId) {
    x = 19;
    y = 29 - 9;
  } else {
    x = -19;
    y = 29 - 9;
  }
  x += _playerSpr->_wx >> 8;
  y += _playerSpr->_wy >> 8;
  for (auto const &sprite : globalRoom->_sprites) {
    if (sprite->_interactAction != ROOMSpriteAction::CUTSCENE)
      continue;
    if (sprite->_spr->_texture == nullptr)
      continue;

    const s32 x2 = sprite->_spr->_wx >> 8;
    const s32 y2 = sprite->_spr->_wy >> 8;
    const s32 w2 = sprite->_spr->_texture->getWidth();
    const s32 h2 = sprite->_spr->_texture->getHeight();

    if (collidesRect(x, y, w, h, x2, y2, w2, h2)) {
      if (globalCutscene == nullptr)
        globalCutscene = std::make_unique<Cutscene>(sprite->_cutsceneId,
                                                    globalRoom->_roomId);
      return;
    }
  }
}

bool Player::check_collisions(s32 dx, s32 dy) const {
  const s32 x = _playerSpr->_wx + dx;
  const s32 y = _playerSpr->_wy + dy;

  for (auto &collider : globalRoom->_roomData.roomColliders.roomColliders) {
    if (!collider.enabled)
      continue;
    if (collidesRect(x >> 8, (y >> 8) + 20, 19, 9, collider.x, collider.y,
                     collider.w, collider.h)) {
      if (collider.colliderAction == 0) // Wall
        return true;
      if (collider.colliderAction == 1 && globalCutscene == nullptr) {
        // Cutscene
        globalCutscene = std::make_unique<Cutscene>(collider.cutsceneId,
                                                    globalRoom->_roomId);
        return true;
      }
    }
  }

  for (auto &roomSprite : globalRoom->_sprites) {
    if (roomSprite->check_player_collide(x, y + (20 << 8), 19 << 8, 9 << 8, dx,
                                         dy))
      return true;
  }

  return false;
}

void Player::draw() {
  _playerSpr->_cam_x = globalCamera._pos->_wx;
  _playerSpr->_cam_y = globalCamera._pos->_wy;
  _playerSpr->_cam_scale_x = globalCamera._pos->_w_scale_x;
  _playerSpr->_cam_scale_y = globalCamera._pos->_w_scale_y;
  _playerSpr->_layer = _playerSpr->_wy >> 8;
}

Player *globalPlayer;
