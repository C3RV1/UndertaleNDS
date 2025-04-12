//
// Created by cervi on 28/08/2022.
//

#include "ManagedSprite.hpp"
#include "Engine/math.hpp"
#include "Formats/ROOM_FILE.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"

void ManagedSprite::load(
    ROOMSprite const &sprData,
    const std::vector<std::shared_ptr<Engine::Texture>> &textures) {
  if (sprData.textureId < textures.size()) {
    _texture = textures[sprData.textureId];
    _spr.loadTexture(*_texture);
  }
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
    _goal_rect_x = sprData.goal_rect_x;
    _goal_rect_y = sprData.goal_rect_y;
    _goal_rect_w = sprData.goal_rect_w;
    _goal_rect_h = sprData.goal_rect_h;
    _cutsceneId = sprData.goal_cutscene_id;
    _goal_flag_id = sprData.goal_flag_id;
    _goal_flag_bit = sprData.goal_flag_bit;
  default:
    break;
  }
}

void ManagedSprite::spawn(
    s8 textureId, s32 x, s32 y,
    const std::vector<std::shared_ptr<Engine::Texture>> &textures) {
  u8 texId2;
  if (textureId < 0)
    texId2 = textures.size() + textureId;
  else
    texId2 = textureId;
  if (texId2 < textures.size()) {
    _texture = textures[texId2];
    _spr.loadTexture(*_texture);
  }
  _spr._wx = x;
  _spr._wy = y;

  _spr.setShown(true);
}

void ManagedSprite::draw(const bool isRoom) {
  if (isRoom) {
    _spr._cam_x = (globalCamera._pos._wx * _parallax_x) >> 8;
    _spr._cam_y = (globalCamera._pos._wy * _parallax_y) >> 8;
    _spr._cam_scale_x = globalCamera._pos._w_scale_x;
    _spr._cam_scale_y = globalCamera._pos._w_scale_y;
    _spr._layer = _spr._wy >> 8;
  }
}

void ManagedSprite::update(const bool isRoom) {
  if (isRoom && _interactAction == ROOMSpriteAction::PROXIMITY) {
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
}

bool ManagedSprite::check_player_collide(s32 x, s32 y, s32 w, s32 h, s32 dx,
                                         s32 dy) {
  if (_interactAction != ROOMSpriteAction::PUSHABLE)
    return false;

  _commit_x = _spr._wx;
  _commit_y = _spr._wy;
  const s32 w2 = _spr._texture->getWidth() << 8;
  const s32 h2 = _spr._texture->getHeight() << 8;

  if (!collidesRect(x, y, w, h, _spr._wx, _spr._wy, w2, h2))
    return false;

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

void ManagedSprite::commit_player_move() {
  if (_interactAction != ROOMSpriteAction::PUSHABLE)
    return;
  _spr._wx = _commit_x;
  _spr._wy = _commit_y;

  // TODO: Goal.
}

void ManagedSprite::free_() { _spr.setShown(false); }
