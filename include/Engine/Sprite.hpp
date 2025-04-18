//
// Created by cervi on 28/08/2022.
//

#ifndef UNDERTALE_SPRITE_HPP
#define UNDERTALE_SPRITE_HPP

namespace Engine {
class Sprite;
}

#include "Engine/OAMManager.hpp"
#include "Engine/Sprite3DManager.hpp"
#include "Engine/Texture.hpp"
#include <nds.h>

namespace Engine {
enum AllocationMode { NoAlloc = 0, AllocatedOAM = 1, Allocated3D = 2 };

class Sprite {
public:
  explicit Sprite(AllocationMode allocMode = NoAlloc);

  void setAnimation(int animId);

  void setFrame(int frameId);

  int nameToAnimId(const std::string &animName) const;

  void tick();

  bool _loaded = false;
  std::shared_ptr<Texture> _texture = nullptr;

  s32 _wx = 0, _wy = 0; // 1 bit sign, 23 bit integer, 8 bit fraction, world
  s32 _w_scale_x = 1 << 8, _w_scale_y = 1 << 8;
  s32 _cam_x = 0, _cam_y = 0;
  s32 _cam_scale_x = 1 << 8, _cam_scale_y = 1 << 8;
  s32 _layer = 0;
  u8 _opacity = 31;

  int _cAnimation = -1;

  friend class OAMManager;
  friend class Sprite3DManager;
  friend void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown);
  friend void spriteSetAllocationMode(std::shared_ptr<Sprite> spr,
                                      AllocationMode allocMode);
  friend void spritePush(std::shared_ptr<Sprite> spr);
  friend void spritePop(std::shared_ptr<Sprite> spr);
  friend void spriteLoadTexture(std::shared_ptr<Sprite> spr,
                                std::shared_ptr<Texture> texture);

  int _cFrame = 0;

private:
  s32 _x = 0, _y = 0; // 1 bit sign, 23 bit integer, 8 bit fraction, screen
  s32 _scale_x = 0, _scale_y = 0;
  u16 _cAnimTimer = 0;
  u16 _cAnimFrame = 0;

  AllocationMode _allocMode;
  AllocationMode _allocated = AllocationMode::NoAlloc;
  bool _shown = false;
  bool _pushed = false;
};

void spriteSetAllocationMode(std::shared_ptr<Sprite> spr,
                             AllocationMode allocMode);
void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown);
void spritePush(std::shared_ptr<Sprite> spr);
void spritePop(std::shared_ptr<Sprite> spr);
void spriteLoadTexture(std::shared_ptr<Sprite> spr, std::string path);
} // namespace Engine

#endif // UNDERTALE_SPRITE_HPP
