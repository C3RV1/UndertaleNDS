//
// Created by cervi on 27/08/26
//

#include "ConditionalFile/RoomConditionalFile.hpp"
#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Formats/utils.hpp"
#include <string>

void RoomSideExit::read(ConditionalReader *cr, SaveData *save) {
  _roomId = readConditionalData<u16>(cr, save);
  auto spawn = readConditionalData<std::tuple<u16, u16>>(cr, save);
  std::tie(_spawnX, _spawnY) = spawn;
  _exitSide = readConditionalData<u8>(cr, save);
}

RoomSideExit readValue(tag<RoomSideExit>, ConditionalReader *cr,
                                  SaveData* save) {
  debug_conditional_file("Reading RoomSideExit value");
  RoomSideExit data;
  data.read(cr, save);
  return data;
}

void RoomSpriteActionUnion::read(ConditionalReader *cr, SaveData* save) {
  cr->rdr->read(&_action, 1);
  switch (_action) {
  case RoomSpriteAction::NONE:
    break;
  case RoomSpriteAction::CUTSCENE:
    _cutscene._cutscene_id = readConditionalData<u16>(cr, save);
    break;
  case RoomSpriteAction::PROXIMITY:
    _proximity._distance = readConditionalData<u16>(cr, save);
    _proximity._close_anim = readConditionalData<std::string>(cr, save);
    break;
  case RoomSpriteAction::PARALLAX: {
    auto parallax = readConditionalData<std::tuple<s32, s32>>(cr, save);
    std::tie(_parallax._parallax_x, _parallax._parallax_y) = parallax;
    break;
  }
  case RoomSpriteAction::PUSHABLE: {
    auto valid_rect = readConditionalData<std::tuple<u16, u16, u16, u16>>(cr, save);
    std::tie(_pushable._valid_rect_x, _pushable._valid_rect_y,
             _pushable._valid_rect_w, _pushable._valid_rect_h) = valid_rect;

    auto goal_pos = readConditionalData<std::tuple<u16, u16>>(cr, save);
    std::tie(_pushable._goal_x, _pushable._goal_y) = goal_pos;

    _pushable._goal_cutscene_id = readConditionalData<u16>(cr, save);
    _pushable._goal_flag_id = readConditionalData<u16>(cr, save);
    _pushable._goal_flag_bit = readConditionalData<u16>(cr, save);
    _pushable._stop_on_goal = readConditionalData<bool>(cr, save);
    break;
  }
  default:
    Engine::throw_("Incorrect room sprite action " + std::to_string((int)_action));
    break;
  }
}

RoomSpriteActionUnion readValue(tag<RoomSpriteActionUnion>, ConditionalReader *cr, SaveData* save) {
  debug_conditional_file("Reading RoomSpriteActionUnion value");
  RoomSpriteActionUnion data;
  data.read(cr, save);
  return data;
}

void RoomSpriteColliderUnion::read(ConditionalReader* cr, SaveData* save) {
  cr->rdr->read(&_hasCollider, 1);
  if (hasCollider()) {
    auto rect = readConditionalData<std::tuple<s8, s8, s8, s8>>(cr, save);
    std::tie(_x, _y, _w, _h) = rect;
  }
}

RoomSpriteColliderUnion readValue(tag<RoomSpriteColliderUnion>, ConditionalReader* cr, SaveData* save) {
  debug_conditional_file("Reading RoomSpriteColliderUnion value");
  RoomSpriteColliderUnion data;
  data.read(cr, save);
  return data;
}

void RoomSpriteData::read(ConditionalReader *cr, SaveData* save) {
  _sprId = readConditionalData<u16>(cr, save);
  _texture = readConditionalData<std::string>(cr, save);
  auto pos = readConditionalData<std::tuple<u16, u16>>(cr, save);
  std::tie(_x, _y) = pos;
  _animation = readConditionalData<std::string>(cr, save);
  _action = readConditionalData<RoomSpriteActionUnion>(cr, save);
  _collider = readConditionalData<RoomSpriteColliderUnion>(cr, save);
}

RoomSpriteData readValue(tag<RoomSpriteData>, ConditionalReader *cr, SaveData* save) {
  debug_conditional_file("Reading RoomSpriteData value");
  RoomSpriteData data;
  data.read(cr, save);
  return data;
}

void RoomColliderTypeUnion::read(ConditionalReader *cr, SaveData* save) {
  cr->rdr->read(&_type, 1);
  switch(_type) {
  case RoomColliderType::WALL:
    break;
  case RoomColliderType::EXIT: {
    _exit._roomId = readConditionalData<u16>(cr, save);
    auto spawn = readConditionalData<std::tuple<u16, u16>>(cr, save);
    std::tie(_exit._spawnX, _exit._spawnY) = spawn;
    break;
  }
  case RoomColliderType::CUTSCENE:
    _cutscene._cutsceneId = readConditionalData<u16>(cr, save);
    break;
  default:
    Engine::throw_("Incorrect room collider type " + std::to_string((int)_type));
    break;
  }
}

RoomColliderTypeUnion readValue(tag<RoomColliderTypeUnion>, ConditionalReader *cr, SaveData* save) {
  debug_conditional_file("Reading RoomColliderTypeUnion value");
  RoomColliderTypeUnion data;
  data.read(cr, save);
  return data;
}

void RoomColliderData::read(ConditionalReader *cr, SaveData *save) {
  _collId = readConditionalData<u8>(cr, save);
  auto rect = readConditionalData<std::tuple<u16, u16, u16, u16>>(cr, save);
  std::tie(_x, _y, _w, _h) = rect;
  _enabled = readConditionalData<bool>(cr, save);

  _type = readConditionalData<RoomColliderTypeUnion>(cr, save);
}

RoomColliderData readValue(tag<RoomColliderData>, ConditionalReader *cr, SaveData* save) {
  debug_conditional_file("Reading RoomColliderData value");
  RoomColliderData data;
  data.read(cr, save);
  return data;
}

void RoomData::read(ConditionalReader *cr, SaveData *save) {
  _roomBg = readConditionalData<std::string>(cr, save);
  _musicPath = readConditionalData<std::string>(cr, save);
  _musicVolume = readConditionalData<u8>(cr, save);
  auto spawn = readConditionalData<std::tuple<u16, u16>>(cr, save);
  std::tie(_spawnX, _spawnY) = spawn;

  _roomExits = VectorConditional<RoomSideExit>::readConditionalVector(cr, save);
  _roomSprites = VectorConditional<RoomSpriteData>::readConditionalVector(cr, save);
  _roomColliders = VectorConditional<RoomColliderData>::readConditionalVector(cr, save);
}

