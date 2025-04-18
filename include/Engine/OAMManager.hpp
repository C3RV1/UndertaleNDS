//
// Created by cervi on 20/08/2022.
//

#ifndef UNDERTALE_OAM_MANAGER_HPP
#define UNDERTALE_OAM_MANAGER_HPP

#include "DEBUG_FLAGS.hpp"
#include "Engine/FreeZoneManager.hpp"
#include "Sprite.hpp"
#include <memory>
#include <nds.h>
#include <vector>

namespace Engine {
struct OAMEntry {
  bool free_ = true;
  u16 tileStart = 0;
  u8 tileWidth = 0, tileHeight = 0;
};

struct SpriteOAMMemory {
  u8 oamScaleIdx = 0xff; // all oam entries can share scale
  std::vector<u8> oamEntries;
  int palette = -1;

  int loadedFrame = -1;
  bool loadedIntoMemory = false;
};

class OAMManager {
public:
  OAMManager(u16 *paletteRam, u16 *tileRam, u16 *oamRam);

#ifdef DEBUG_2D
  void dumpOamState();
#endif

  void draw();

private:
  friend void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown);

  int loadSprite(std::shared_ptr<Sprite> res);
  void freeSprite(std::shared_ptr<Sprite> spr);
  void freeSpriteData(SpriteOAMMemory &mem);

  int reserveOAMEntry(u8 tileW, u8 tileH);
  void freeOAMEntry(int oamId);

  void setSpritePosAndScale(Sprite &spr, SpriteOAMMemory &mem);
  int loadSpriteFrame(Sprite &spr, SpriteOAMMemory &mem, int frame);
  void setOAMState(SpriteOAMMemory &mem);
  void allocateOamScaleEntry(SpriteOAMMemory &mem);
  void freeOamScaleEntry(SpriteOAMMemory &spr);

  u16 *_paletteRam;
  u16 *_oamRam;
  u16 *_tileRam;

  FreeZoneManager _tileZones;
  std::vector<std::pair<std::weak_ptr<Sprite>, SpriteOAMMemory>> _activeSpr;

  u8 _paletteRefCounts[16] = {0};
  OAMEntry _oamEntries[SPRITE_COUNT];
  bool _oamScaleEntryUsed[32] = {false};
};

extern OAMManager OAMManagerSub;
} // namespace Engine

#endif // UNDERTALE_OAM_MANAGER_HPP
