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



class RoomSpriteData : public ConditionalObj {
public:
  
private:
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

private:
};

#endif
