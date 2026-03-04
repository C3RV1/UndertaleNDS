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
  _spr->_wx += dx;
  _spr->_wy += dy;

  // Push objects if necessary.
  for (auto &roomSprite : globalRoom->_sprites) {
    roomSprite->commit_player_move();
  }
}

void Player::check_exits() {
  u16 width, height;
  globalRoom->_bg.getSize(width, height);
  if (_spr->_wx < 0) {
    _spr->_wx = 0;
    if (globalRoom->_exitLeft != nullptr) {
      loadNewRoom(globalRoom->_exitLeft->roomId, globalRoom->_exitLeft->spawnX,
                  globalRoom->_exitLeft->spawnY);
    }
  } else if ((_spr->_wx >> 8) + 19 > width) {
    _spr->_wx = (width - 19) << 8;
    if (globalRoom->_exitRight != nullptr) {
      loadNewRoom(globalRoom->_exitRight->roomId,
                  globalRoom->_exitRight->spawnX,
                  globalRoom->_exitRight->spawnY);
    }
  }
  if (_spr->_wy < 0) {
    _spr->_wy = 0;
    if (globalRoom->_exitTop != nullptr) {
      loadNewRoom(globalRoom->_exitTop->roomId, globalRoom->_exitTop->spawnX,
                  globalRoom->_exitTop->spawnY);
    }
  } else if ((_spr->_wy >> 8) + 29 > height) {
    _spr->_wy = (height - 29) << 8;
    if (globalRoom->_exitBtm != nullptr) {
      loadNewRoom(globalRoom->_exitBtm->roomId, globalRoom->_exitBtm->spawnX,
                  globalRoom->_exitBtm->spawnY);
    }
  }

  for (int i = 0; i < globalRoom->_rectExitCount; i++) {
    ROOMExit *rectExit = globalRoom->_rectExits[i];
    if (collidesRect(_spr->_wx >> 8, (_spr->_wy >> 8) + 20, 19, 9, rectExit->x,
                     rectExit->y, rectExit->w, rectExit->h)) {
      loadNewRoom(rectExit->roomId, rectExit->spawnX, rectExit->spawnY);
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
  const s32 x = _spr->_wx + dx;
  const s32 y = _spr->_wy + dy;

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
  _spr->_cam_x = globalCamera._pos->_wx;
  _spr->_cam_y = globalCamera._pos->_wy;
  _spr->_cam_scale_x = globalCamera._pos->_w_scale_x;
  _spr->_cam_scale_y = globalCamera._pos->_w_scale_y;
  _spr->_layer = _spr->_wy >> 8;
}

Player *globalPlayer;
