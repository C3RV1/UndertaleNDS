//
// Created by cervi on 27/08/2022.
//

#include "Camera.hpp"

void Camera::updatePosition(bool roomChange) {
    int xTilePrev = (prevX >> 8) / 8, yTilePrev = (prevY >> 8) / 8;
    if (!manual) {
        pos.wx = globalPlayer->spriteManager.wx - ((256 / 2 - 9) << 8);
        pos.wy = globalPlayer->spriteManager.wy - ((192 / 2 - 14) << 8);
    }
    uint16_t roomW, roomH;
    globalRoom->bg.getSize(roomW, roomH);
    if ((pos.wx >> 8) + 256 > roomW * 8) {
        pos.wx = (roomW * 8 - 256) << 8;
    }
    if (pos.wx < 0) {
        pos.wx = 0;
    }
    if ((pos.wy >> 8) + 192 > roomH * 8) {
        pos.wy = (roomH * 8 - 192) << 8;
    }
    if (pos.wy < 0) {
        pos.wy = 0;
    }
    if (!roomChange) {
        // 3D engine lags behind 1 frame for some reason
        if (prevX != pos.wx || prevY != pos.wy) {
            REG_BG3X = mod(pos.wx, 512 << 8);
            REG_BG3Y = mod(pos.wy, 512 << 8);
        }
    } else {
        REG_BG3X = mod(pos.wx, 512 << 8);
        REG_BG3Y = mod(pos.wy, 512 << 8);
    }
    REG_BG3PA = (1 << 16) / pos.wscale_x;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3PD = (1 << 16) / pos.wscale_y;
    int xTilePost = (pos.wx >> 8) / 8, yTilePost = (pos.wy >> 8) / 8;
    if ((xTilePrev != xTilePost || yTilePrev != yTilePost) && !roomChange) {
        int incrementX = xTilePost > xTilePrev ? 1 : -1;
        int incrementY = yTilePost > yTilePrev ? 1 : -1;
        for (int xTile = xTilePrev; xTile != xTilePost; xTile += incrementX) {
            Engine::loadBgRectMain(globalRoom->bg, xTile + incrementX + 32, yTilePost - 1, 1, 26);
            Engine::loadBgRectMain(globalRoom->bg, xTile + incrementX - 1, yTilePost - 1, 1, 26);
        }
        for (int yTile = yTilePrev; yTile != yTilePost; yTile += incrementY) {
            Engine::loadBgRectMain(globalRoom->bg, xTilePost - 1, yTile + incrementY + 24, 34, 1);
            Engine::loadBgRectMain(globalRoom->bg, xTilePost - 1, yTile + incrementY - 1, 34, 1);
        }
    } else if (roomChange) {
        Engine::loadBgRectMain(globalRoom->bg, xTilePost - 1, yTilePost - 1, 34, 26);
    }
    prevX = pos.wx, prevY = pos.wy;
}

Camera globalCamera;
