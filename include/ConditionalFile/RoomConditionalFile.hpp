//
// Created by cervi on 27/08/26
//
#ifndef UNDERTALE_ROOM_CONDITIONAL_FILE_HPP
#define UNDERTALE_ROOM_CONDITIONAL_FILE_HPP

#include "ConditionalFile.hpp"
#include <string>

struct RoomHeader {
  char header[4] = {'R', 'O', 'O', 'M'};
  u32 fileSize = 0;

  u32 version = 11;
  static constexpr u32 version_expected = 10;
};

class RoomSideExit : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

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

  RoomSpriteAction _action = RoomSpriteAction::NONE;

  struct {
    // Type Cutscene
    u16 _cutscene_id;
  } _cutscene;
  
  struct {
    // Type Proximity
    u16 _distance;
    std::string _close_anim;
  } _proximity;

  struct {
    // Type Parallax
    s32 _parallax_x, _parallax_y;
  } _parallax;

  struct {
    // Type Pushable
    u16 _valid_rect_x, _valid_rect_y, _valid_rect_w, _valid_rect_h;
    u16 _goal_x, _goal_y;
    u16 _goal_cutscene_id;
    u16 _goal_flag_id;
    u16 _goal_flag_bit;
    bool _stop_on_goal;
  } _pushable;
};
RoomSpriteActionUnion readConditionalValue(tag<RoomSpriteActionUnion>, BufferReader* rdr, SaveData* save);


class RoomSpriteData : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;
  
  u16 _sprId;
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

  RoomColliderType _type = RoomColliderType::WALL;
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

  u8 _collId;
  u16 _x, _y, _w, _h;
  bool _enabled;

  RoomColliderTypeUnion _type;
};
RoomColliderData readConditionalValue(tag<RoomColliderData>, BufferReader* rdr, SaveData* save);


class RoomData : public ConditionalObj {
public:
  void read(BufferReader* rdr, SaveData* save) override;

  std::string _roomBg;
  std::string _musicPath;
  u8 _musicVolume;
  u16 _spawnX, _spawnY;

  std::vector<RoomSideExit> _roomExits;
  std::vector<RoomSpriteData> _roomSprites;
  std::vector<RoomColliderData> _roomColliders;
};

#endif
