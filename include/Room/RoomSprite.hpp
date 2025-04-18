//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_MANAGED_SPRITE_HPP
#define UNDERTALE_MANAGED_SPRITE_HPP

#include <memory>
class RoomSprite;

#include "Engine/Sprite.hpp"
#include "Formats/ROOM_FILE.hpp"

class RoomSprite {
public:
  explicit RoomSprite(Engine::AllocationMode alloc) : _spr(alloc) {}

  void load(ROOMSpriteData const &sprData);

  void spawn(s32 x, s32 y, std::shared_ptr<Engine::Texture> texture);

  bool check_player_collide(s32 x, s32 y, s32 w, s32 h, s32 dx, s32 dy);
  void commit_player_move();

  void update(bool isRoom);

  void draw(bool isRoom);

  Engine::Sprite _spr;

  ROOMSpriteAction _interactAction = ROOMSpriteAction::NONE;

  u16 _cutsceneId = 0;

  u32 _distanceSquared = 0;
  int _closeAnim = 0;

  int _animationId = 0;
  s32 _parallax_x = 1 << 8;
  s32 _parallax_y = 1 << 8;

  u16 _valid_rect_x, _valid_rect_y, _valid_rect_w, _valid_rect_h;
  u16 _goal_x, _goal_y;
  u16 _goal_flag_id;
  u16 _goal_flag_bit;
  bool _stop_on_goal;
  s32 _commit_x, _commit_y;

private:
  bool check_on_goal();

  void free_();
};

#endif // UNDERTALE_MANAGED_SPRITE_HPP
