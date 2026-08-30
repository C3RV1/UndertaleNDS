//
// Created by cervi on 27/08/2022.
//

#include "Room/Camera.hpp"
#include "Engine/math.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"

void Camera::updatePosition(Player& player) {
  constexpr int offsetX = 9, offsetY = 14 - 20;
  if (!_manual) {
    _pos->_wx = player._spr->_wx + (offsetX << 8);
    _pos->_wy = player._spr->_wy + (offsetY << 8);
  }
  
  s32 sx = _pos->_w_scale_x, sy = _pos->_w_scale_y;
  s32 ex = edgeDistanceX(), ey = edgeDistanceY();

  u16 rw, rh;
  _room->_bg.getSize(rw, rh);
  s32 roomW = (s32)rw << 8, roomH = (s32)rh << 8;
  if ((roomW * sx) >> 8 >= 256 << 8) {
    if (_pos->_wx + ex > roomW)
      _pos->_wx = roomW - ex;
    if (_pos->_wx < ex)
      _pos->_wx = ex;
  } else {
    _pos->_wx = roomW / 2;
  }
  if ((roomH * sy) >> 8 >= 192 << 8) {
    if (_pos->_wy + ey > roomH)
      _pos->_wy = roomH - ey;
    if (_pos->_wy < ey)
      _pos->_wy = ey;
  } else {
    _pos->_wy = roomH / 2;
  }
  
  Engine::bg3ScrollX = mod(_pos->_wx - ex, 512 << 8);
  Engine::bg3ScrollY = mod(_pos->_wy - ey, 512 << 8);

  Engine::bg3Pa = (1 << 16) / _pos->_w_scale_x;
  Engine::bg3Pb = 0;
  Engine::bg3Pc = 0;
  Engine::bg3Pd = (1 << 16) / _pos->_w_scale_y;
}

void Camera::drawBackground(bool roomChange) {
  int xTilePrev = (_prevX >> 8) / 8, yTilePrev = (_prevY >> 8) / 8;
  int xTilePost = ((_pos->_wx - edgeDistanceX()) >> 8) / 8,
      yTilePost = ((_pos->_wy - edgeDistanceY()) >> 8) / 8;
  if (roomChange) {
    _room->_bg.loadBgRectMain(xTilePost - 1, yTilePost - 1, 34, 26);
  } else if (xTilePrev != xTilePost || yTilePrev != yTilePost) {
    int incrementX = xTilePost > xTilePrev ? 1 : -1;
    int incrementY = yTilePost > yTilePrev ? 1 : -1;
    for (int xTile = xTilePrev; xTile != xTilePost; xTile += incrementX) {
      if (incrementX > 0)
        _room->_bg.loadBgRectMain(xTile + incrementX + 32, yTilePost - 1, 1,
                                  26);
      else
        _room->_bg.loadBgRectMain(xTile + incrementX - 1, yTilePost - 1, 1, 26);
    }
    for (int yTile = yTilePrev; yTile != yTilePost; yTile += incrementY) {
      if (incrementY > 0)
        _room->_bg.loadBgRectMain(xTilePost - 1, yTile + incrementY + 24, 34,
                                  1);
      else
        _room->_bg.loadBgRectMain(xTilePost - 1, yTile + incrementY - 1, 34, 1);
    }
  }
  _prevX = _pos->_wx - edgeDistanceX(), _prevY = _pos->_wy - edgeDistanceY();
}

