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
  }
  void drawGlyph(Font &font, u8 glyph, int &x, int y);
  void reloadColors();
  void setPaletteColor(int colorIdx, int r, int g, int b, bool color8bit);
  void setPaletteColor(int colorIdx, u16 color5bit);
  void setColor(int colorIdx) { _paletteColor = colorIdx; }
  u16 getColor() const { return _paletteColor; }
  void clear();
  void clearRect(int x, int y, int w, int h);

  void drawRect(int x, int y, int w, int h, int colorIdx);
  void drawHpBar(int hp, int maxHp, int x, int y, int w, int h);
  void drawHollowRect(int x, int y, int w, int h, int width, int colorIdx);

  void tick();

private:
  u8 *getTile(int x, int y);
  void updateDirty(u32 localTileId);

  u16 *_paletteRam;
  u16 *_tileRam;
  u16 *_mapRam;
  int _tileReserve = 1;
  int _paletteColor = 15;

  static constexpr u32 TILE_BUFFER_SIZE = 64;

  bool _dirty[TILE_BUFFER_SIZE] = {false};
  u16 _tileIds[TILE_BUFFER_SIZE] = {0};
  u8 _tiles[TILE_BUFFER_SIZE][32] = {0};
};

extern TextBGManager textMain;
extern TextBGManager textSub;
} // namespace Engine

#endif
