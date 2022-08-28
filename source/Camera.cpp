//
// Created by cervi on 27/08/2022.
//

#include "Camera.hpp"

void Camera::updatePosition(Room &room, Player &player, bool roomChange) {
    int xTilePrev = (x >> 8) / 8, yTilePrev = (y >> 8) / 8;
    int prevX = x, prevY = y;
    x = player.x - ((256 / 2 - 9) << 8);
    y = player.y - ((192 / 2 - 14) << 8);
    uint8_t roomW, roomH;
    room.bg.getSize(roomW, roomH);
    if ((x >> 8) + 256 > roomW * 8) {
        x = (roomW * 8 - 256) << 8;
    }
    if (x < 0) {
        x = 0;
    }
    if ((y >> 8) + 192 > roomH * 8) {
        y = (roomH * 8 - 192) << 8;
    }
    if (y < 0) {
        y = 0;
    }
    if (!roomChange) {
        // 3D engine lags behind 1 frame for some reason
        REG_BG3X = mod(prevX, 512 << 8);
        REG_BG3Y = mod(prevY, 512 << 8);
    } else {
        REG_BG3X = mod(x, 512 << 8);
        REG_BG3Y = mod(y, 512 << 8);
    }
    int xTilePost = (x >> 8) / 8, yTilePost = (y >> 8) / 8;
    if ((xTilePrev != xTilePost || yTilePrev != yTilePost) && !roomChange) {
        Engine::loadBgRectMain(room.bg, xTilePost + 32, yTilePost, 1, 26);
        Engine::loadBgRectMain(room.bg, xTilePost - 1, yTilePost, 1, 26);
        Engine::loadBgRectMain(room.bg, xTilePost, yTilePost + 24, 34, 1);
        Engine::loadBgRectMain(room.bg, xTilePost, yTilePost - 1, 34, 1);
    } else if (roomChange) {
        Engine::loadBgRectMain(room.bg, xTilePost - 1, yTilePost - 1, 34, 26);
    }
}
