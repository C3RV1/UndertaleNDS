//
// Created by cervi on 28/08/2022.
//

#include "Room/RoomSprite.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Texture.hpp"
#include "Engine/math.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include "Save.hpp"
#include <memory>

void RoomSprite::load(ROOMSpriteData const &sprData) {
  _spr.loadTexture(Engine::textureManager.loadTexture(sprData.path));
  _animationId = _spr.nameToAnimId(sprData.animation);
  _spr._wx = sprData.x << 8;
  _spr._wy = sprData.y << 8;
  _spr.setAnimation(_animationId);

  _spr.setShown(true);

  _interactAction = static_cast<ROOMSpriteAction>(sprData.interactAction);
  _parallax_x = 1 << 8;
  _parallax_y = 1 << 8;

  switch (_interactAction) {
  case ROOMSpriteAction::CUTSCENE:
    _cutsceneId = sprData.cutsceneId;
    break;
  case ROOMSpriteAction::PROXIMITY:
    _distanceSquared = sprData.distance * sprData.distance;
    _closeAnim = _spr.nameToAnimId(sprData.closeAnim);
    break;
  case ROOMSpriteAction::PARALLAX:
    _parallax_x = sprData.parallax_x;
    _parallax_y = sprData.parallax_y;
    break;
  case ROOMSpriteAction::PUSHABLE:
    _valid_rect_x = sprData.valid_rect_x;
    _valid_rect_y = sprData.valid_rect_y;
    _valid_rect_w = sprData.valid_rect_w;
    _valid_rect_h = sprData.valid_rect_h;
    _goal_x = sprData.goal_x;
    _goal_y = sprData.goal_y;
    _cutsceneId = sprData.goal_cutscene_id;
    _goal_flag_id = sprData.goal_flag_id;
    _goal_flag_bit = sprData.goal_flag_bit;
    _stop_on_goal = sprData.stop_on_goal;
  default:
    break;
  }
}

void RoomSprite::spawn(s32 x, s32 y, std::shared_ptr<Engine::Texture> texture) {
  _spr.loadTexture(std::move(texture));
  _spr._wx = x;
  _spr._wy = y;

  _spr.setShown(true);
}

void RoomSprite::draw() {
  _spr._cam_x = (globalCamera._pos._wx * _parallax_x) >> 8;
  _spr._cam_y = (globalCamera._pos._wy * _parallax_y) >> 8;
  _spr._cam_scale_x = globalCamera._pos._w_scale_x;
  _spr._cam_scale_y = globalCamera._pos._w_scale_y;
  _spr._layer = _spr._wy >> 8;
}

void RoomSprite::update() {
  if (_interactAction != ROOMSpriteAction::PROXIMITY)
    return;

  if (_spr._texture == nullptr)
    return;
  if (globalPlayer->_playerSpr._texture == nullptr)
    return;
  const u16 width = _spr._texture->getWidth();
  const u16 height = _spr._texture->getHeight();
  const u16 pw = globalPlayer->_playerSpr._texture->getWidth();
  const u16 ph = globalPlayer->_playerSpr._texture->getHeight();
  const u32 distance =
      distSquared_fp(_spr._wx + width / 2, _spr._wy + height / 2,
                     globalPlayer->_playerSpr._wx + pw / 2,
                     globalPlayer->_playerSpr._wy + ph / 2);
  if (distance >> 8 < _distanceSquared)
    _spr.setAnimation(_closeAnim);
  else
    _spr.setAnimation(_animationId);
}

bool RoomSprite::check_player_collide(s32 x, s32 y, s32 w, s32 h, s32 dx,
                                      s32 dy) {
  if (_interactAction != ROOMSpriteAction::PUSHABLE)
    return false;

  _commit_x = _spr._wx;
  _commit_y = _spr._wy;
  const s32 w2 = _spr._texture->getWidth() << 8;
  const s32 h2 = _spr._texture->getHeight() << 8;

  if (!collidesRect(x, y, w, h, _spr._wx, _spr._wy, w2, h2))
    return false;
  if (check_on_goal() && _stop_on_goal)
    return true;

  if (!collidesRect(x, y, w, h, _spr._wx + dx, _spr._wy, w2, h2)) {
    // Attempt move x-axis
    if (!rectContainsOther(_valid_rect_x, _valid_rect_y, _valid_rect_w,
                           _valid_rect_h, (_spr._wx + dx) >> 8, _spr._wy >> 8,
                           w2 >> 8, h2 >> 8))
      return true;
    _commit_x = _spr._wx + dx;
    return false;
  }

  if (!collidesRect(x, y, w, h, _spr._wx, _spr._wy + dy, w2, h2)) {
    // Attempt move y-axis
    if (!rectContainsOther(_valid_rect_x, _valid_rect_y, _valid_rect_w,
                           _valid_rect_h, _spr._wx >> 8, (_spr._wy + dy) >> 8,
                           w2 >> 8, h2 >> 8))
      return true;
    _commit_y = _spr._wy + dy;
    return false;
  }

  if (!collidesRect(x, y, w, h, _spr._wx + dx, _spr._wy + dy, w2, h2)) {
    // Attempt move both axes
    if (!rectContainsOther(_valid_rect_x, _valid_rect_y, _valid_rect_w,
                           _valid_rect_h, (_spr._wx + dx) >> 8,
                           (_spr._wy + dy) >> 8, w2 >> 8, h2 >> 8))
      return true;
    _commit_x = _spr._wx + dx;
    _commit_y = _spr._wy + dy;
    return false;
  }

  return false;
}

void RoomSprite::commit_player_move() {
  if (_interactAction != ROOMSpriteAction::PUSHABLE)
    return;
  bool flag_set = (globalSave.flags[_goal_flag_id] & _goal_flag_bit) != 0;
  _spr._wx = _commit_x;
  _spr._wy = _commit_y;

  bool should_set_flag = check_on_goal();
  if (should_set_flag)
    globalSave.flags[_goal_flag_id] |= _goal_flag_bit;
  else
    globalSave.flags[_goal_flag_id] &= ~_goal_flag_bit;

  if (should_set_flag != flag_set) {
    if (should_set_flag && _stop_on_goal) {
      _spr._wx = _goal_x << 8;
      _spr._wy = _goal_y << 8;
    }

    if (globalCutscene == nullptr)
      globalCutscene =
          std::make_unique<Cutscene>(_cutsceneId, globalRoom->_roomId);
    else
      nocashMessage("Cannot create goal cutscene: Already playing another!");
  }
}

bool RoomSprite::check_on_goal() {
  s32 dx = _spr._wx - ((s32)_goal_x << 8);
  if (dx < 0)
    dx = -dx;
  s32 dy = _spr._wy - ((s32)_goal_y << 8);
  if (dy < 0)
    dy = -dy;

  return dx <= 2 << 8 && dy <= 2 << 8;
}

void RoomSprite::free_() { _spr.setShown(false); }
