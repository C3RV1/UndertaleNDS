//
// Created by cervi on 12/04/2025
//

#ifndef UNDERTALE_TEXTBGMANAGER_HPP
#define UNDERTALE_TEXTBGMANAGER_HPP

#include "Engine/Font.hpp"
#include <nds.h>

namespace Engine {
class TextBGManager {
public:
  TextBGManager(u16 *paletteRam, u16 *tileRam, u16 *mapRam)
      : _paletteRam(paletteRam), _tileRam(tileRam), _mapRam(mapRam) {
    reloadColors();
    void resetTileReserve();
  }
  void drawGlyph(Font &font, u8 glyph, int &x, int y);
  void reloadColors();
  void setPaletteColor(u8 colorIdx, int r, int g, int b, bool color8bit);
  void setPaletteColor(u8 colorIdx, u16 color5bit);
  void setColor(u8 colorIdx) { _paletteColor = colorIdx & 0xF; }
  u16 getColor() const { return _paletteColor; }
  void clear();
  inline void clearRect(int x, int y, int w, int h) {
    drawRect(x, y, w, h, 0);
  }

  void drawRect(int x, int y, int w, int h, u8 colorIdx);
  void drawHpBar(int hp, int maxHp, int x, int y, int w, int h);
  void drawHollowRect(int x, int y, int w, int h, int width, u8 colorIdx);

  void drawToVRAM();

private:
  enum DIRTY_TILE_UPDATE {
    DIRTY_NONE = 0,
    DIRTY_COPY = 1,
    DIRTY_CLEAR = 2
  };
  
  u8 *getTile(int x, int y);
  void clearTile(int x, int y);
  void resetTileReserve();
  void updateDirty(u32 localTileId);

  u16 *_paletteRam;
  u16 *_tileRam;
  u16 *_mapRam;
  u16 _tileReserve[32 * 24 - 1];
  u16 _tileFront = 0;
  u8 _paletteColor = 15;

  static constexpr u32 TILE_BUFFER_SIZE = 24 * 32;

  u8 _dirty[TILE_BUFFER_SIZE] = {DIRTY_NONE};
  u16 _tileVramIds[TILE_BUFFER_SIZE] = {0};
  u8 _tiles[TILE_BUFFER_SIZE][32] = {0};

  inline u16 tilePosToIdx(int tileX, int tileY) {
    return (tileY * 32 + tileX);
  }
};

extern TextBGManager textMain;
extern TextBGManager textSub;
} // namespace Engine

#endif
