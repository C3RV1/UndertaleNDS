//
// Created by cervi on 27/08/2022.
//

#include "Room/Camera.hpp"
#include "Engine/math.hpp"
#include "Room/Room.hpp"
#include "Room/Player.hpp"

void Camera::updatePosition(bool roomChange) {
    constexpr int offsetX = 0, offsetY = -20;
    int xTilePrev = (_prevX >> 8) / 8, yTilePrev = (_prevY >> 8) / 8;
    if (!_manual) {
        _pos._wx = globalPlayer->_playerSpr._wx - ((256 / 2 - 9) << 8) + (offsetX << 8);
        _pos._wy = globalPlayer->_playerSpr._wy - ((192 / 2 - 14) << 8) + (offsetY << 8);
    }
    u16 roomW, roomH;
    globalRoom->_bg.getSize(roomW, roomH);
    if (roomW >= 256) {
        if ((_pos._wx >> 8) + 256 > roomW) {
            _pos._wx = (roomW - 256) << 8;
        }
        if (_pos._wx < 0) {
            _pos._wx = 0;
        }
    }
    else {
        _pos._wx = (roomW / 2 - 128) << 8;
    }
    if (roomH >= 192) {
        if ((_pos._wy >> 8) + 192 > roomH) {
            _pos._wy = (roomH - 192) << 8;
        }
        if (_pos._wy < 0) {
            _pos._wy = 0;
        }
    }
    else {
        _pos._wy = (roomH / 2 - 96) << 8;
    }

    Engine::bg3ScrollX = mod(_pos._wx, 512 << 8);
    Engine::bg3ScrollY = mod(_pos._wy, 512 << 8);

    Engine::bg3Pa = (1 << 16) / _pos._w_scale_x;
    Engine::bg3Pb = 0;
    Engine::bg3Pc = 0;
    Engine::bg3Pd = (1 << 16) / _pos._w_scale_y;

    int xTilePost = (_pos._wx >> 8) / 8, yTilePost = (_pos._wy >> 8) / 8;
    if (roomChange) {
        globalRoom->_bg.loadBgRectMain(xTilePost - 1, yTilePost - 1, 34, 26);
    }
    else if (xTilePrev != xTilePost || yTilePrev != yTilePost) {
        int incrementX = xTilePost > xTilePrev ? 1 : -1;
        int incrementY = yTilePost > yTilePrev ? 1 : -1;
        for (int xTile = xTilePrev; xTile != xTilePost; xTile += incrementX) {
            globalRoom->_bg.loadBgRectMain(xTile + incrementX + 32, yTilePost - 1, 1, 26);
            globalRoom->_bg.loadBgRectMain(xTile + incrementX - 1, yTilePost - 1, 1, 26);
        }
        for (int yTile = yTilePrev; yTile != yTilePost; yTile += incrementY) {
            globalRoom->_bg.loadBgRectMain(xTilePost - 1, yTile + incrementY + 24, 34, 1);
            globalRoom->_bg.loadBgRectMain(xTilePost - 1, yTile + incrementY - 1, 34, 1);
        }
    }
    _prevX = _pos._wx, _prevY = _pos._wy;
}

Camera globalCamera;
