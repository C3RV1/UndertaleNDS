//
// Created by cervi on 28/08/2022.
//

#include "Room/RoomSprite.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/math.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include <memory>

void RoomSprite::load(RoomSpriteData const &sprData) {
  Engine::spriteLoadTexture(_spr, sprData._texture);
  _animation_id = _spr->nameToAnimId(sprData._animation);
  _spr->_wx = sprData._x << 8;
  _spr->_wy = sprData._y << 8;
  _spr->setAnimation(_animation_id);

  Engine::spriteSetShown(_spr, true);

  const RoomSpriteActionUnion &a = sprData._action;
  _action = a._action;

  switch (_action) {
  case RoomSpriteAction::CUTSCENE:
    _cutscene._cutscene_id = a._cutscene._cutscene_id;
    break;
  case RoomSpriteAction::PROXIMITY: {
    u16 d = a._proximity._distance;
    _proximity._distanceSquared = d * d;
    _proximity._closeAnim = _spr->nameToAnimId(a._proximity._close_anim);
    break;
  }
  case RoomSpriteAction::PARALLAX:
    _parallax._parallax_x = a._parallax._parallax_x;
    _parallax._parallax_y = a._parallax._parallax_y;
    break;
  case RoomSpriteAction::PUSHABLE:
    _pushable._valid_rect_x = a._pushable._valid_rect_x;
    _pushable._valid_rect_y = a._pushable._valid_rect_y;
    _pushable._valid_rect_w = a._pushable._valid_rect_w;
    _pushable._valid_rect_h = a._pushable._valid_rect_h;
    _pushable._goal_x = a._pushable._goal_x;
    _pushable._goal_y = a._pushable._goal_y;
    _pushable._goal_cutscene_id = a._pushable._goal_cutscene_id;
    _pushable._goal_flag_id = a._pushable._goal_flag_id;
    _pushable._goal_flag_bit = a._pushable._goal_flag_bit;
    _pushable._stop_on_goal = a._pushable._stop_on_goal;
  default:
    break;
  }
}

void RoomSprite::spawn(s32 x, s32 y, std::string path) {
  Engine::spriteLoadTexture(_spr, path);
  _spr->_wx = x;
  _spr->_wy = y;

  Engine::spriteSetShown(_spr, true);
}

void RoomSprite::draw() {
  _spr->_cam_x = _room->_camera._pos->_wx;
  _spr->_cam_y = _room->_camera._pos->_wy;
  if (_action == RoomSpriteAction::PARALLAX) {
    _spr->_cam_x *= _parallax._parallax_x;
    _spr->_cam_x >>= 8;
    _spr->_cam_y *= _parallax._parallax_y;
    _spr->_cam_y >>= 8;
  }
  
  _spr->_cam_scale_x = _room->_camera._pos->_w_scale_x;
  _spr->_cam_scale_y = _room->_camera._pos->_w_scale_y;
  _spr->_layer = _spr->_wy >> 8;
}

void RoomSprite::update() {
  switch (_action) {
  case RoomSpriteAction::PROXIMITY:
    updateProximity();
    break;
  case RoomSpriteAction::PUSHABLE:
    updatePushable();
    break;
  default:
    break;
  }
}

void RoomSprite::updateProximity() {
  if (_spr->_texture == nullptr)
    return;
  if (_room->_player._spr->_texture == nullptr)
    return;
  const u16 width = _spr->_texture->getWidth();
  const u16 height = _spr->_texture->getHeight();
  const u16 pw = _room->_player._spr->_texture->getWidth();
  const u16 ph = _room->_player._spr->_texture->getHeight();
  const u32 distance = distSquared_fp(
      _spr->_wx + width / 2, _spr->_wy + height / 2,
      _room->_player._spr->_wx + pw / 2, _room->_player._spr->_wy + ph / 2);
  if (distance >> 8 < _proximity._distanceSquared)
    _spr->setAnimation(_proximity._closeAnim);
  else
    _spr->setAnimation(_animation_id);
}

void RoomSprite::updatePushable() {
  // Runs after Navigation, so any position changes can already be detected.
  // If after Navigation we colide with the Player, we must have moved,
  // not the Player. We should try to push the Player, then, if we collide
  // with them.
  s32 x = _spr->_wx;
  s32 y = _spr->_wy;
  s32 dx = x - _pushable._old_x;
  s32 dy = y - _pushable._old_y;
  if (dx == 0 && dy == 0)
    return;

  _pushable._old_x = _spr->_wx;
  _pushable._old_y = _spr->_wy;

  s32 px = _room->_player._spr->_wx;
  s32 py = _room->_player._spr->_wy;
  s32 pw = _room->_player._spr->_texture->getWidth() << 8;
  s32 ph = _room->_player._spr->_texture->getHeight() << 8;

  s32 w = _spr->_texture->getWidth() << 8;
  s32 h = _spr->_texture->getHeight() << 8;

  if (!collidesRect(px, py, pw, ph, x, y, w, h))
    return;

  // Push player
  _room->_player._spr->_wx += dx;
  _room->_player._spr->_wy += dy;
}

bool RoomSprite::check_player_collide(s32 x, s32 y, s32 w, s32 h, s32 dx,
                                      s32 dy) {
  if (_action != RoomSpriteAction::PUSHABLE)
    return false;

  _pushable._commit_x = _spr->_wx;
  _pushable._commit_y = _spr->_wy;
  const s32 w2 = _spr->_texture->getWidth() << 8;
  const s32 h2 = _spr->_texture->getHeight() << 8;

  if (!collidesRect(x, y, w, h, _spr->_wx, _spr->_wy, w2, h2))
    return false;
  if (check_on_goal() && _pushable._stop_on_goal)
    return true;

  if (!collidesRect(x, y, w, h, _spr->_wx + dx, _spr->_wy, w2, h2)) {
    // Attempt move x-axis
    if (!rectContainsOther(_pushable._valid_rect_x, _pushable._valid_rect_y,
                           _pushable._valid_rect_w, _pushable._valid_rect_h,
                           (_spr->_wx + dx) >> 8, _spr->_wy >> 8,
                           w2 >> 8, h2 >> 8))
      return true;
    _pushable._commit_x = _spr->_wx + dx;
    return false;
  }

  if (!collidesRect(x, y, w, h, _spr->_wx, _spr->_wy + dy, w2, h2)) {
    // Attempt move y-axis
    if (!rectContainsOther(_pushable._valid_rect_x, _pushable._valid_rect_y,
                           _pushable._valid_rect_w, _pushable._valid_rect_h,
                           _spr->_wx >> 8, (_spr->_wy + dy) >> 8, w2 >> 8,
                           h2 >> 8))
      return true;
    _pushable._commit_y = _spr->_wy + dy;
    return false;
  }

  if (!collidesRect(x, y, w, h, _spr->_wx + dx, _spr->_wy + dy, w2, h2)) {
    // Attempt move both axes
    if (!rectContainsOther(_pushable._valid_rect_x, _pushable._valid_rect_y,
                           _pushable._valid_rect_w, _pushable._valid_rect_h,
                           (_spr->_wx + dx) >> 8, (_spr->_wy + dy) >> 8,
                           w2 >> 8, h2 >> 8))
      return true;
    _pushable._commit_x = _spr->_wx + dx;
    _pushable._commit_y = _spr->_wy + dy;
    return false;
  }

  return false;
}

void RoomSprite::commit_player_move() {
  if (_action != RoomSpriteAction::PUSHABLE)
    return;
  bool flag_set = (_room->_save->flags[_pushable._goal_flag_id] &
                   _pushable._goal_flag_bit) != 0;
  _spr->_wx = _pushable._commit_x;
  _spr->_wy = _pushable._commit_y;
  _pushable._old_x = _spr->_wx;
  _pushable._old_y = _spr->_wy;

  bool should_set_flag = check_on_goal();
  if (should_set_flag)
    _room->_save->flags[_pushable._goal_flag_id] |= _pushable._goal_flag_bit;
  else
    _room->_save->flags[_pushable._goal_flag_id] &= ~_pushable._goal_flag_bit;

  if (should_set_flag != flag_set) {
    if (should_set_flag && _pushable._stop_on_goal) {
      _spr->_wx = _pushable._goal_x << 8;
      _spr->_wy = _pushable._goal_y << 8;
    }

    if (_room->_cutscene == nullptr)
      _room->_cutscene = std::make_unique<Cutscene>(_pushable._goal_cutscene_id,
                                                    _room->_roomId, _room);
    else
      Engine::log_("Cannot create goal cutscene: Already playing another!");
  }
}

bool RoomSprite::check_on_goal() {
  s32 dx = _spr->_wx - ((s32)_pushable._goal_x << 8);
  if (dx < 0)
    dx = -dx;
  s32 dy = _spr->_wy - ((s32)_pushable._goal_y << 8);
  if (dy < 0)
    dy = -dy;

  return dx <= 2 << 8 && dy <= 2 << 8;
}
