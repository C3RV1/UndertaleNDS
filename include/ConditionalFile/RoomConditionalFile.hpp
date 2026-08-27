//
// Created by cervi on 27/08/26
//
#ifndef UNDERTALE_ROOM_CONDITIONAL_FILE_HPP
#define UNDERTALE_ROOM_CONDITIONAL_FILE_HPP

#include "ConditionalFile.hpp"
#include <string>


class RoomSideExit : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

private:
  u16 _roomId;
  u16 _spawnX, _spawnY;
  u8 _exitSide;
};
RoomSideExit readConditionalValue(tag<RoomSideExit>, BufferReader* rdr, SaveData* save);

enum class RoomSpriteAction {
  NONE = 0,
  CUTSCENE = 1,
  PROXIMITY = 2,
  PARALLAX = 3,
  PUSHABLE = 4
};

class RoomSpriteActionUnion : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

private:
  RoomSpriteAction _action;
  union {
    struct {
      // Type Cutscene
      u16 _cutsceneId;
    } _cutscene;
    struct {
      // Type Proximity
      u16 _distance;
    } _proximity;
    struct {
      // Type Parallax
      s32 _parallaxX, _parallaxY;
    } _parallax;
    struct {
      // Type Pushable
      u16 _validRectX, _validRectY, _validRectW, _validRectH;
      u16 _goalPosX, _goalPosY;
      u16 _cutsceneId;
      u16 _goalFlagId;
      u16 _goalFlagBit;
      bool _stopOnGoal;
    } _pushable;
  };
};
RoomSpriteActionUnion readConditionalValue(tag<RoomSpriteActionUnion>, BufferReader* rdr, SaveData* save);


class RoomSpriteData : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;
  
private:
  s16 _sprId;
  std::string _texture;
  u16 _x, _y;
  std::string _animation;
  RoomSpriteActionUnion _action;
};
RoomSpriteData readConditionalValue(tag<RoomSpriteData>, BufferReader* rdr, SaveData* save);


enum class RoomColliderType {
  WALL = 0,
  EXIT = 1,
  CUTSCENE = 2
};


class RoomColliderTypeUnion : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

private:
  RoomColliderType _type;
  union {
    struct {
      // Type Exit
      u16 _roomId;
      u16 _spawnX, _spawnY;
    } _exit;
    struct {
      // Type Cutscene
      u16 _cutsceneId;
    } _cutscene;
  };
};
RoomColliderTypeUnion readConditionalValue(tag<RoomColliderTypeUnion>, BufferReader* rdr, SaveData* save);


class RoomColliderData : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

private:
  s16 _collId;
  u16 _rectX, _rectY, _rectW, _rectH;
  bool _enabled;

  RoomColliderTypeUnion _type;
};
RoomColliderData readConditionalValue(tag<RoomColliderData>, BufferReader* rdr, SaveData* save);


class RoomData : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

private:
  std::string _roomBg;
  std::string _musicPath;
  u8 _musicVolume;
  u16 _spawnX, _spawnY;

  std::vector<RoomSideExit> _roomExits;
  std::vector<RoomSpriteData> _roomSprites;
  std::vector<RoomColliderData> _roomColliders;
};

#endif
