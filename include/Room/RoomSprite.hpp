//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_MANAGED_SPRITE_HPP
#define UNDERTALE_MANAGED_SPRITE_HPP

#include <memory>
class RoomSprite;

#include "Engine/Sprite.hpp"
#include "Room/Camera.hpp"
#include "ConditionalFile/RoomConditionalFile.hpp"

class Room;

class RoomSprite {
public:
  RoomSprite(Engine::AllocationMode alloc, Room* room) : _room(room) {
    _spr = std::make_shared<Engine::Sprite>(alloc);
  }

  void load(RoomSpriteData const &sprData);

  void spawn(s32 x, s32 y, std::string path);

  bool check_player_collide(s32 x, s32 y, s32 w, s32 h, s32 dx, s32 dy);
  void commit_player_move();

  void update();
  void updateProximity();
  void updateColliderPush();

  void updateDrawPositions(Camera &cam);

  std::shared_ptr<Engine::Sprite> _spr;

  RoomSpriteAction _action = RoomSpriteAction::NONE;
  
  int _animation_id = 0;
  
  struct {
    u16 _cutscene_id = 0;
  } _cutscene;
  
  struct {
    u32 _distanceSquared = 0;
    int _closeAnim = 0;
  } _proximity;
  
  struct {
    s32 _parallax_x = 1 << 8;
    s32 _parallax_y = 1 << 8;
  } _parallax;
  
  struct {
    u16 _valid_rect_x, _valid_rect_y, _valid_rect_w, _valid_rect_h;
    u16 _goal_x, _goal_y;
    u16 _goal_cutscene_id;
    u16 _goal_flag_id;
    u16 _goal_flag_bit;
    bool _stop_on_goal;
    s32 _commit_x, _commit_y;
  } _pushable;

  bool _hasCollider;
  s8 _coll_x, _coll_y, _coll_w, _coll_h;
  
private:
  s32 _old_x, _old_y;

  Room* _room;
  bool check_player_collide_pushable(s32 x, s32 y, s32 w, s32 h, s32 dx,
                                     s32 dy);
  bool check_on_goal();
};

#endif // UNDERTALE_MANAGED_SPRITE_HPP
