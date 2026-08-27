//
// Created by cervi on 27/08/26
//

#include "ConditionalFile/RoomConditionalFile.hpp"
#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
#include <string>

void RoomSideExit::read(BufferReader* rdr, SaveData *save) {
  _roomId = readConditionalData<u16>(rdr, save, this);
  _spawnX = readConditionalData<u16>(rdr, save, this);
  _spawnY = readConditionalData<u16>(rdr, save, this);
  _exitSide = readConditionalData<u8>(rdr, save, this);
}

RoomSideExit readConditionalValue(tag<RoomSideExit>, BufferReader* rdr,
                                  SaveData* save) {
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
    _cutscene._cutsceneId = readConditionalData<u16>(rdr, save, this);
    break;
  case RoomSpriteAction::PROXIMITY:
    _proximity._distance = readConditionalData<u16>(rdr, save, this);
    break;
  case RoomSpriteAction::PARALLAX:
    _parallax._parallaxX = readConditionalData<s32>(rdr, save, this);
    _parallax._parallaxY = readConditionalData<s32>(rdr, save, this);
    break;
  case RoomSpriteAction::PUSHABLE:
    _pushable._validRectX = readConditionalData<u16>(rdr, save, this);
    _pushable._validRectY = readConditionalData<u16>(rdr, save, this);
    _pushable._validRectW = readConditionalData<u16>(rdr, save, this);
    _pushable._validRectH = readConditionalData<u16>(rdr, save, this);
    _pushable._goalPosX = readConditionalData<u16>(rdr, save, this);
    _pushable._goalPosY = readConditionalData<u16>(rdr, save, this);
    _pushable._cutsceneId = readConditionalData<u16>(rdr, save, this);
    _pushable._goalFlagId = readConditionalData<u16>(rdr, save, this);
    _pushable._goalFlagBit = readConditionalData<u16>(rdr, save, this);
    _pushable._stopOnGoal = readConditionalData<bool>(rdr, save, this);
    break;
  default:
    Engine::throw_("Incorrect room sprite action " + std::to_string((int)_action));
    break;
  }
}

RoomSpriteActionUnion readConditionalValue(tag<RoomSpriteActionUnion>, BufferReader* rdr, SaveData* save) {
  RoomSpriteActionUnion data;
  data.read(rdr, save);
  return data;
}

void RoomSpriteData::read(BufferReader* rdr, SaveData* save) {
  _sprId = readConditionalData<s16>(rdr, save, this);
  _texture = readConditionalData<std::string>(rdr, save, this);
  _x = readConditionalData<u16>(rdr, save, this);
  _y = readConditionalData<u16>(rdr, save, this);
  _animation = readConditionalData<std::string>(rdr, save, this);
  _action = readConditionalData<RoomSpriteActionUnion>(rdr, save, this);
}

RoomSpriteData readConditionalValue(tag<RoomSpriteData>, BufferReader* rdr, SaveData* save) {
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

RoomColliderTypeUnion readConditionalValue(tag<RoomColliderTypeUnion>, BufferReader* rdr, SaveData* save) {
  RoomColliderTypeUnion data;
  data.read(rdr, save);
  return data;
}

void RoomColliderData::read(BufferReader *rdr, SaveData *save) {
  _collId = readConditionalData<s16>(rdr, save, this);
  _rectX = readConditionalData<u16>(rdr, save, this);
  _rectY = readConditionalData<u16>(rdr, save, this);
  _rectW = readConditionalData<u16>(rdr, save, this);
  _rectH = readConditionalData<u16>(rdr, save, this);
  _enabled = readConditionalData<bool>(rdr, save, this);

  _type = readConditionalData<RoomColliderTypeUnion>(rdr, save, this);
}

RoomColliderData readConditionalValue(tag<RoomColliderData>, BufferReader* rdr, SaveData* save) {
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

