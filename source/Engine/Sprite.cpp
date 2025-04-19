//
// Created by cervi on 30/08/2022.
//
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include <memory>
#include <string>

namespace Engine {
Sprite::Sprite(Engine::AllocationMode allocMode) { _allocMode = allocMode; }

void spriteSetAllocationMode(std::shared_ptr<Sprite> spr,
                             Engine::AllocationMode allocMode) {
  if (allocMode == spr->_allocMode)
    return;
  bool lastShown = spr->_shown;
  spriteSetShown(spr, false);
  spr->_allocMode = allocMode;
  spriteSetShown(spr, lastShown);
}

void Sprite::setAnimation(int animId) {
  if (!_loaded)
    return;
  if (animId >= _texture->_animationCount || animId < 0)
    return;
  if (_cAnimation == animId)
    return;
  _cAnimation = animId;
  _cAnimFrame = 0;
  _cAnimTimer = _texture->_animations[animId].frames[0].duration;
  _cFrame = _texture->_animations[animId].frames[0].frame;
}

void spriteLoadTexture(std::shared_ptr<Sprite> spr, std::string path) {
  if (!spr)
    return;
  std::shared_ptr<Texture> texture = textureManager.loadTexture(path);
  if (!texture->getLoaded())
    return;
  if (texture == spr->_texture)
    return;
  spritePush(spr);

  spr->_texture = std::move(texture);
  spr->_loaded = true;
  spr->_cFrame = 0;
  spr->_cAnimation = -1;

  int animId = spr->nameToAnimId("gfx"); // default animation
  if (animId != -1)
    spr->setAnimation(animId);
  spritePop(spr);
}

ITCM_CODE
void Sprite::tick() {
  if (!_loaded)
    return;
  if (_cAnimation >= 0) {
    CSPRAnimation *current = &_texture->_animations[_cAnimation];
    if (current->frames[_cAnimFrame].duration != 0) {
      _cAnimTimer--;
      if (_cAnimTimer == 0) {
        _cAnimFrame++;
        _cAnimFrame %= current->frames.size();
        _cFrame = current->frames[_cAnimFrame].frame;
        _cAnimTimer = current->frames[_cAnimFrame].duration;
      }
    }
  }

  _x = _wx - _cam_x;
  _y = _wy - _cam_y;
  if (_cAnimation >= 0) {
    CSPRAnimation *current = &_texture->_animations[_cAnimation];
    CSPRAnimFrame *frameInfo = &current->frames[_cAnimFrame];
    _x += frameInfo->drawOffX << 8;
    _y += frameInfo->drawOffY << 8;
  }
  _x *= _cam_scale_x;
  _x >>= 8;
  _y *= _cam_scale_y;
  _y >>= 8;
  _scale_x = (_cam_scale_x * _w_scale_x) >> 8;
  _scale_y = (_cam_scale_y * _w_scale_y) >> 8;
}

void Sprite::setFrame(int frameId) {
  if (!_loaded)
    return;
  _cAnimation = -1;
  _cFrame = frameId;
}

void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown) {
  if (!spr)
    return;
  if (!spr->_loaded)
    return;
  if (shown == spr->_shown)
    return;
  spr->_shown = shown;
  if (spr->_shown) {
    if (spr->_allocMode == Allocated3D)
      main3dSpr.loadSprite(spr);
    else if (spr->_allocMode == AllocatedOAM) {
      OAMManagerSub.loadSprite(spr);
    }
  } else {
    if (spr->_allocMode == Allocated3D)
      main3dSpr.freeSprite(spr);
    else if (spr->_allocMode == AllocatedOAM)
      OAMManagerSub.freeSprite(spr);
  }
}

int Sprite::nameToAnimId(const std::string &animName) const {
  if (!_loaded)
    return -1;
  for (int i = 0; i < _texture->_animationCount; i++) {
    if (animName == _texture->_animations[i].name) {
      return i;
    }
  }
  std::string buffer = "Not found animation: " + animName;
  nocashMessage(buffer.c_str());
  return -1;
}

void spritePush(std::shared_ptr<Sprite> spr) {
  if (!spr)
    return;
  spr->_pushed = spr->_shown;
  spriteSetShown(spr, false);
}

void spritePop(std::shared_ptr<Sprite> spr) {
  if (!spr)
    return;
  spriteSetShown(spr, spr->_pushed);
}
} // namespace Engine
