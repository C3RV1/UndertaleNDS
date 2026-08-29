//
// Created by cervi on 27/08/26
//

#include "ConditionalFile/RoomConditionalFile.hpp"
#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Formats/utils.hpp"
#include <string>

void RoomSideExit::read(BufferReader* rdr, SaveData *save) {
  _roomId = readConditionalData<u16>(rdr, save, this);
  _spawnX = readConditionalData<u16>(rdr, save, this);
  _spawnY = readConditionalData<u16>(rdr, save, this);
  _exitSide = readConditionalData<u8>(rdr, save, this);
}

RoomSideExit readValue(tag<RoomSideExit>, BufferReader* rdr,
                                  SaveData* save) {
  debug_conditional_file("Reading RoomSideExit value");
  RoomSideExit data;
  data.read(rdr, save);
  return data;
}

void RoomSpriteActionUnion::read(BufferReader* rdr, SaveData* save) {
  rdr->read(&_action, 1);
  switch (_action) {
  case RoomSpriteAction::NONE:
    break;
  case RoomSpriteAction::CUTSCENE:
    _cutscene._cutscene_id = readConditionalData<u16>(rdr, save, this);
    break;
  case RoomSpriteAction::PROXIMITY:
    _proximity._distance = readConditionalData<u16>(rdr, save, this);
    _proximity._close_anim = readConditionalData<std::string>(rdr, save, this);
    break;
  case RoomSpriteAction::PARALLAX:
    _parallax._parallax_x = readConditionalData<s32>(rdr, save, this);
    _parallax._parallax_y = readConditionalData<s32>(rdr, save, this);
    break;
  case RoomSpriteAction::PUSHABLE:
    _pushable._valid_rect_x = readConditionalData<u16>(rdr, save, this);
    _pushable._valid_rect_y = readConditionalData<u16>(rdr, save, this);
    _pushable._valid_rect_w = readConditionalData<u16>(rdr, save, this);
    _pushable._valid_rect_h = readConditionalData<u16>(rdr, save, this);
    _pushable._goal_x = readConditionalData<u16>(rdr, save, this);
    _pushable._goal_y = readConditionalData<u16>(rdr, save, this);
    _pushable._goal_cutscene_id = readConditionalData<u16>(rdr, save, this);
    _pushable._goal_flag_id = readConditionalData<u16>(rdr, save, this);
    _pushable._goal_flag_bit = readConditionalData<u16>(rdr, save, this);
    _pushable._stop_on_goal = readConditionalData<bool>(rdr, save, this);
    break;
  default:
    Engine::throw_("Incorrect room sprite action " + std::to_string((int)_action));
    break;
  }
}

RoomSpriteActionUnion readValue(tag<RoomSpriteActionUnion>, BufferReader* rdr, SaveData* save) {
  debug_conditional_file("Reading RoomSpriteActionUnion value");
  RoomSpriteActionUnion data;
  data.read(rdr, save);
  return data;
}

void RoomSpriteData::read(BufferReader* rdr, SaveData* save) {
  _sprId = readConditionalData<u16>(rdr, save, this);
  _texture = readConditionalData<std::string>(rdr, save, this);
  _x = readConditionalData<u16>(rdr, save, this);
  _y = readConditionalData<u16>(rdr, save, this);
  _animation = readConditionalData<std::string>(rdr, save, this);
  _action = readConditionalData<RoomSpriteActionUnion>(rdr, save, this);
}

RoomSpriteData readValue(tag<RoomSpriteData>, BufferReader* rdr, SaveData* save) {
  debug_conditional_file("Reading RoomSpriteData value");
  RoomSpriteData data;
  data.read(rdr, save);
  return data;
}

void RoomColliderTypeUnion::read(BufferReader* rdr, SaveData* save) {
  rdr->read(&_type, 1);
  switch(_type) {
  case RoomColliderType::WALL:
    break;
  case RoomColliderType::EXIT:
    _exit._roomId = readConditionalData<u16>(rdr, save, this);
    _exit._spawnX = readConditionalData<u16>(rdr, save, this);
    _exit._spawnY = readConditionalData<u16>(rdr, save, this);
    break;
  case RoomColliderType::CUTSCENE:
    _cutscene._cutsceneId = readConditionalData<u16>(rdr, save, this);
    break;
  default:
    Engine::throw_("Incorrect room collider type " + std::to_string((int)_type));
    break;
  }
}

RoomColliderTypeUnion readValue(tag<RoomColliderTypeUnion>, BufferReader* rdr, SaveData* save) {
  debug_conditional_file("Reading RoomColliderTypeUnion value");
  RoomColliderTypeUnion data;
  data.read(rdr, save);
  return data;
}

void RoomColliderData::read(BufferReader *rdr, SaveData *save) {
  _collId = readConditionalData<u8>(rdr, save, this);
  _x = readConditionalData<u16>(rdr, save, this);
  _y = readConditionalData<u16>(rdr, save, this);
  _w = readConditionalData<u16>(rdr, save, this);
  _h = readConditionalData<u16>(rdr, save, this);
  _enabled = readConditionalData<bool>(rdr, save, this);

  _type = readConditionalData<RoomColliderTypeUnion>(rdr, save, this);
}

RoomColliderData readValue(tag<RoomColliderData>, BufferReader* rdr, SaveData* save) {
  debug_conditional_file("Reading RoomColliderData value");
  RoomColliderData data;
  data.read(rdr, save);
  return data;
}

void RoomData::read(BufferReader *rdr, SaveData *save) {
  _roomBg = readConditionalData<std::string>(rdr, save, this);
  _musicPath = readConditionalData<std::string>(rdr, save, this);
  _musicVolume = readConditionalData<u8>(rdr, save, this);
  _spawnX = readConditionalData<u16>(rdr, save, this);
  _spawnY = readConditionalData<u16>(rdr, save, this);

  _roomExits = VectorConditional<RoomSideExit>::readConditionalVector(rdr, save);
  _roomSprites = VectorConditional<RoomSpriteData>::readConditionalVector(rdr, save);
  _roomColliders = VectorConditional<RoomColliderData>::readConditionalVector(rdr, save);
}

