//
// Created by cervi on 12/04/2025
//

#include "Engine/TextBGManager.hpp"
#include "Engine/dma.hpp"
#include <cstring>

namespace Engine {
void TextBGManager::drawGlyph(Font &font, u8 glyph, int &x, int y) {
  if (!font._loaded)
    return;

  u8 glyphIdx = font._glyphMap.glyphMap[glyph];
  if (glyphIdx == 0)
    return;
  const CFNTGlyph *glyphObj = font.getGlyph(glyphIdx);
  int endX = x + glyphObj->shift;
  x += glyphObj->offset;

  // Until we haven't completed the glyph in the y-axis
  for (u8 glyphY = 0; glyphY < glyphObj->height && y < 192;) {

    // Copy x, when we go back to the start of the line we want to keep the
    // starting x
    int x_ = x;

    // Until we haven't completed the glyph in the x-axis
    // in the current vertical strip (tiles are 8x8, vertical strip are all
    //                                tiles on the same row)
    for (u8 glyphX = 0; glyphX < glyphObj->width && x_ < 256;) {
      // Pointer to the current tile
      u8 *tilePointer = getTile(x_, y);

      // Offset in current tile
      u8 tileX = x_ % 8;
      u8 tileY = y % 8;

      // Copy the current glyph y position
      int glyphY_ = glyphY;

      // Until we haven't completed the glyph in the current tile vertically
      for (; tileY < 8 && y + tileY < 192 && glyphY_ < glyphObj->height;
           tileY++) {

        // Copy tile x and glyph x
        // When we go back to the start of the tile we want to keep these
        int tileX_ = tileX;
        int glyphX_ = glyphX;

        // Until we haven't completed the glyph in the current tile horizontally
        for (; tileX_ < 8 && x_ + tileX_ < 256 && glyphX_ < glyphObj->width;
             tileX_++) {
          // Get pointer to the current pixel (can only write in words, so we
          // round to the word (& (~1)). Then we'll shift the bits accordingly).
          u8 *tileByte = tilePointer + (((tileY * 8 + tileX_) / 2) & (~1));
          auto *tile = (u16 *)tileByte;

          // Are we writing to the high bits of the byte (0xX0) or low bits
          // (0x0X)?
          bool highBits = (tileX_ & 1) == 1;

          // Are we writing to the byte coming before (0xXX00) or memory of
          // after (0x00XX) (because we can only write in words)?
          bool prevByte = (((tileY * 8 + tileX_) / 2) & 1) == 1;

          // Position of glyph bit (glyphs are 1 bit depth)
          u32 bitPos = glyphY_ * glyphObj->width + glyphX_;
          u32 byte = bitPos / 8;
          bitPos = 7 - (bitPos % 8);

          // Get the pixel of the glyph as the lowest significant bit
          // (0b0000000x)
          u8 bit = glyphObj->glyphData[byte] >> bitPos;

          // Clear tile position.
          *tile &= ~(0xF << (4 * highBits) << (8 * prevByte));
          if (bit & 1) {
            // Write palette color
            *tile |= _paletteColor << (4 * highBits) << (8 * prevByte);
          }
          glyphX_++;
        }
        glyphY_++;
      }

      // Move to next tile in row
      glyphX += 8 - (x_ % 8);
      x_ += 8 - (x_ % 8);
    }

    // Move to next vertical strip
    glyphY += 8 - (y % 8);
    y += 8 - (y % 8);
  }
  x = endX;
}

void TextBGManager::reloadColors() {
  _paletteRam[16 * 15 + 0] = 31 << 5;          // full green color (transparent)
  _paletteRam[16 * 15 + 7] = 31 + (31 << 5);   // yellow - spare color
  _paletteRam[16 * 15 + 8] = 0;                // black color
  _paletteRam[16 * 15 + 9] = 31;               // full red color
  _paletteRam[16 * 15 + 10] = 31 << 5;         // full green color
  _paletteRam[16 * 15 + 11] = 31 << 10;        // full blue color
  _paletteRam[16 * 15 + 12] = 31 + (31 << 5);  // red + green = yellow
  _paletteRam[16 * 15 + 13] = 31 + (31 << 10); // red + blue = purple
  _paletteRam[16 * 15 + 14] =
      (31 << 5) + (31 << 10); // green + blue = turquoise
  _paletteRam[16 * 15 + 15] = (31 << 10) + (31 << 5) + 31; // full white color
}

void TextBGManager::clear() {
  dmaFillSafe(3, 0, _mapRam, 2 * 32 * 32);
  _tileReserve = 1;
  for (u32 i = 0; i < TILE_BUFFER_SIZE; i++)
    _tileIds[i] = 0;
}

void TextBGManager::clearRect(int x, int y, int w, int h) {
  drawRect(x, y, w, h, 0);
}

u8 *TextBGManager::getTile(int x, int y) {
  x /= 8;
  y /= 8;
  u16 tileId = *((u8 *)_mapRam + (y * 32 + x) * 2) & 0x1FF;

  u16 innerTileId = (x + 32 * (y % 2)) % TILE_BUFFER_SIZE;

  if (tileId == 0) {
    tileId = _tileReserve++;
    *(u16 *)((u8 *)_mapRam + (y * 32 + x) * 2) = (15 << 12) + tileId;
    // Initialize tile to blank
    dmaFillSafe(3, 0, ((u8 *)_tileRam) + (tileId * 32), 32);

    updateDirty(innerTileId);
    _tileIds[innerTileId] = tileId;
    memset(_tiles[innerTileId], 0, 32);
  } else if (_tileIds[innerTileId] != tileId) {
    updateDirty(innerTileId);
    _tileIds[innerTileId] = tileId;

    memcpy(_tiles[innerTileId], (u8 *)_tileRam + (tileId * 32), 32);
  }

  _dirty[innerTileId] = true;
  return _tiles[innerTileId];
}

void TextBGManager::updateDirty(u32 localTileId) {
  if (!_dirty[localTileId] || _tileIds[localTileId] == 0)
    return;

  dmaCopySafe(3, _tiles[localTileId],
              (u8 *)_tileRam + (_tileIds[localTileId] * 32), 32);
  _dirty[localTileId] = false;
}

void TextBGManager::tick() {
  for (int i = 0; i < TILE_BUFFER_SIZE; i++)
    updateDirty(i);
  while (dmaBusy(3))
    ;
}

void TextBGManager::setPaletteColor(int colorIdx, int r, int g, int b,
                                    bool color8bit) {
  if (color8bit) {
    r >>= 3;
    g >>= 3;
    b >>= 3;
  }
  _paletteRam[16 * 15 + colorIdx] = (b << 10) + (g << 5) + r;
}

void TextBGManager::setPaletteColor(int colorIdx, u16 color5bit) {
  _paletteRam[16 * 15 + colorIdx] = color5bit;
}

void TextBGManager::drawRect(int x, int y, int w, int h, int colorIdx) {
  // Look at drawGlyph for an explanation on this code
  // as it follows the same idea.
  if (x < 0) {
    w -= -x;
    x = 0;
  }
  if (y < 0) {
    h -= -y;
    y = 0;
  }
  if (x + w > 256)
    w = 256 - x;
  if (y + h > 192)
    h = 192 - y;

  int dstX = x + w;
  int dstY = y + h;
  for (; y < dstY;) {
    int x_ = x;
    for (; x_ < dstX;) {
      u8 *tilePointer = getTile(x_, y);
      u8 tileY = y % 8;
      u8 tileX = x_ % 8;

      if (tileX == 0 && tileY == 0 && x_ + 8 < x + w && y + 8 < dstY) {
        x_ += 8;
        // Each tile is 4 bits.
        dmaFillSafe(3, colorIdx * 0x11111111, tilePointer, 32);
        continue;
      }
      for (; tileY < 8 && y + tileY < dstY; tileY++) {
        int tileX_ = tileX;
        for (; tileX_ < 8 && x_ + tileX_ < dstX; tileX_++) {
          u8 *tileByte = tilePointer + (((tileY * 8 + tileX_) / 2) & (~1));
          auto *tile = (u16 *)tileByte;

          bool highBits = (tileX_ & 1) == 1;
          bool prevByte = (((tileY * 8 + tileX_) / 2) & 1) == 1;

          *tile &= ~(0xF << (4 * highBits + 8 * prevByte));
          *tile |= colorIdx << (4 * highBits + 8 * prevByte);
        }
      }
      x_ += 8 - (x_ % 8);
    }
    y += 8 - (y % 8);
  }
}

void TextBGManager::drawHollowRect(int x, int y, int w, int h, int width,
                                   int colorIdx) {
  // Look at drawGlyph for an explanation on this code
  // as it follows the same idea.
  if (x < 0) {
    w -= -x;
    x = 0;
  }
  if (y < 0) {
    h -= -y;
    y = 0;
  }
  if (x + w > 256)
    w = 256 - x;
  if (y + h > 192)
    h = 192 - y;

  int width_w = width;
  int width_h = width;
  if (width > w)
    width_w = w;
  if (width > h)
    width_h = h;

  drawRect(x, y, w, width_h, colorIdx);
  drawRect(x, y, width_w, h, colorIdx);
  drawRect(x, y + h - width_h, w, width_h, colorIdx);
  drawRect(x + w - width_w, y, width_w, h, colorIdx);
}

void TextBGManager::drawHpBar(int hp, int maxHp, int x, int y, int w, int h) {
  int green_w = (hp * w) / maxHp; // Floor(w * (hp/maxHp))
  int red_w = w - green_w;
  drawRect(x, y, green_w, h, 12);        // Draw yellow.
  drawRect(x + green_w, y, red_w, h, 9); // Draw red.
}

TextBGManager textMain(BG_PALETTE, BG_TILE_RAM(5), BG_MAP_RAM(4));
TextBGManager textSub(BG_PALETTE_SUB, BG_TILE_RAM_SUB(5), BG_MAP_RAM_SUB(1));
} // namespace Engine
