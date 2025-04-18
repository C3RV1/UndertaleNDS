#include "Engine/OAMManager.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/dma.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Engine {
OAMManager::OAMManager(u16 *paletteRam, u16 *tileRam, u16 *oamRam)
    : _paletteRam(paletteRam), _oamRam(oamRam), _tileRam(tileRam),
      _tileZones(0, 4096, "2D_TILES") {
  for (int i = 0; i < 16; i++) {
    *(paletteRam + i * 16) = 31 << 5; // full green for bg
  }
  for (int oamId = 0; oamId < SPRITE_COUNT; oamId++) {
    auto *oamStart = (u16 *)((u8 *)_oamRam + oamId * 8);
    oamStart[0] = 1 << 9;
  }
}

int OAMManager::loadSprite(std::shared_ptr<Sprite> res) {
  if (!res->_loaded)
    return -1;
  if (res->_allocated != NoAlloc)
    return -2;
  if (!res->_texture->_hasOam) {
    std::string buffer = "Error loading spr #r" + res->_texture->_path +
                         "#x to OAM: Sprite doesn't have OAM chunk.";
    throw_(buffer);
  }

  auto &colors = res->_texture->_colors;

  SpriteOAMMemory mem;

  mem.palette = -1;
  int freePalette = -1;
  for (int paletteIdx = 0; paletteIdx < 16 && mem.palette == -1; paletteIdx++) {
    // If we have found a free palette, store it
    if (_paletteRefCounts[paletteIdx] == 0 && freePalette == -1) {
      freePalette = paletteIdx;
      continue;
    }

    // If not, check if the color match
    // If they do, we can reuse the palette
    mem.palette = paletteIdx;
    for (size_t i = 0; i < res->_texture->_colors.size(); i++) {
      if (_paletteRam[1 + paletteIdx * 16 + i] != colors[i]) {
        mem.palette = -1;
        break;
      }
    }
  }

  if (mem.palette == -1) {
    // If we haven't found a palette we can reuse
    // then we use the free palette
    if (freePalette == -1) {
      // If we couldn't find a free palette, return.
      std::string buffer = "Error loading spr #r" + res->_texture->_path +
                           "#x to OAM: No available palettes.";
      throw_(buffer);
    }
    mem.palette = freePalette;
    dmaCopySafe(3, &res->_texture->_colors[0],
                &_paletteRam[1 + freePalette * 16],
                res->_texture->_colors.size() * 2);
  }
  _paletteRefCounts[mem.palette]++;

  // Reserve oam tiles
  u8 oamW = res->_texture->_oamChunk.oamW;
  u8 oamH = res->_texture->_oamChunk.oamH;
  mem.oamEntries.resize(oamW * oamH);

  for (int oamY = 0; oamY < oamH; oamY++) {
    for (int oamX = 0; oamX < oamW; oamX++) {
      auto &oamEntry = res->_texture->_oamChunk.oamEntries[oamY * oamW + oamX];
      int oamId = reserveOAMEntry(oamEntry.tilesW, oamEntry.tilesH);
      mem.oamEntries[oamY * oamW + oamX] = oamId;
    }
  }

  mem.loadedFrame = -1;
  mem.loadedIntoMemory = false;
  _activeSpr.push_back(std::make_pair(res, mem));
  res->_allocated = AllocatedOAM;
  return 0;
}

int OAMManager::loadSpriteFrame(Sprite &spr, SpriteOAMMemory &mem, int frame) {
  if (mem.loadedFrame == frame)
    return -1;
  if (frame >= spr._texture->_frameCount || frame < 0)
    return -2;

  mem.loadedFrame = frame;
  u8 oamW = spr._texture->_oamChunk.oamW;
  u8 oamH = spr._texture->_oamChunk.oamH;

  // Copy tile into memory
  for (int oamY = 0; oamY < oamH; oamY++) {
    for (int oamX = 0; oamX < oamW; oamX++) {
      auto &textureOamEntry =
          spr._texture->_oamChunk.oamEntries[oamY * oamW + oamX];

      int oamId = mem.oamEntries[oamY * oamW + oamX];
      auto &oamEntry = _oamEntries[oamId];
      u16 *tileRamStart = (u16 *)((u8 *)_tileRam + oamEntry.tileStart * 32);
      u32 tileBytes = textureOamEntry.tilesW * textureOamEntry.tilesH * 32;
      u32 frameOffset = tileBytes * frame;
      u8 *tileStart = &textureOamEntry.tilesFrameData[0] + frameOffset;

      dmaCopySafe(3, tileStart, tileRamStart, tileBytes);
    }
  }
  return 0;
}

int OAMManager::reserveOAMEntry(u8 tileW, u8 tileH) {
  int oamId = -1;
  OAMEntry *oamEntry = nullptr;
  for (int i = SPRITE_COUNT - 1; i > 0; i--) {
    if (_oamEntries[i].free_) {
      oamId = i;
      oamEntry = &_oamEntries[i];
      break;
    }
  }
  if (oamId == -1) {
    std::string buffer = "Error reserving OAM entry: OAM is full";
    throw_(buffer);
  }
  oamEntry->free_ = false;
  oamEntry->tileWidth = tileW;
  oamEntry->tileHeight = tileH;

  // load tiles in groups of animations
  u16 neededTiles = oamEntry->tileWidth * oamEntry->tileHeight;
  _tileZones.reserve(neededTiles, oamEntry->tileStart, 2);
#ifdef DEBUG_2D
  dumpOamState();
#endif

  return oamId;
}

void OAMManager::freeOAMEntry(int oamId) {
  if (_oamEntries[oamId].free_)
    return;
  _oamEntries[oamId].free_ = true;

  OAMEntry *oamEntry = &_oamEntries[oamId];

  auto *oamStart = (u16 *)((u8 *)_oamRam + oamId * 8);
  oamStart[0] = 1 << 9; // Not displayed
  oamStart[1] = 0;
  oamStart[2] = 0;

  u16 length = oamEntry->tileWidth * oamEntry->tileHeight;
  _tileZones.free(length, oamEntry->tileStart);

#ifdef DEBUG_2D
  dumpOamState();
#endif
}

#ifdef DEBUG_2D
void OAMManager::dumpOamState() {
  char buffer[100];
  for (int i = 0; i < 128; i++) {
    if (_oamEntries[i].free_)
      continue;
    sprintf(buffer, "OAM %d start %d w %d h %d", i, _oamEntries[i].tileStart,
            _oamEntries[i].tileWidth, _oamEntries[i].tileHeight);
    nocashMessage(buffer);
  }
}
#endif

void OAMManager::freeSprite(std::shared_ptr<Sprite> spr) {
  if (spr->_allocated != AllocatedOAM)
    return;
  auto sprIdx = _activeSpr.begin();
  for (; sprIdx != _activeSpr.end(); sprIdx++) {
    if (sprIdx->first.lock() == spr)
      break;
  }
  if (sprIdx == _activeSpr.end())
    return;
  SpriteOAMMemory &mem = sprIdx->second;
  freeSpriteData(mem);

  _activeSpr.erase(sprIdx);
  spr->_allocated = NoAlloc;
}

void OAMManager::freeSpriteData(SpriteOAMMemory &mem) {
  _paletteRefCounts[mem.palette]--;

  if (mem.oamScaleIdx != 0xff) {
    freeOamScaleEntry(mem);
    mem.oamScaleIdx = 0xff;
  }
  for (auto const &oamId : mem.oamEntries) {
    freeOAMEntry(oamId);
  }
  mem.oamEntries.clear();
}

void OAMManager::draw() {
  if (_activeSpr.empty())
    return;
  for (auto i = _activeSpr.begin(); i != _activeSpr.end();) {
    if (i->first.expired()) {
      freeSpriteData(i->second);
      i = _activeSpr.erase(i);
      continue;
    }

    std::shared_ptr<Sprite> spr = i->first.lock();
    SpriteOAMMemory &mem = i->second;
    i++;

    spr->tick();

    if (spr->_cFrame != mem.loadedFrame)
      loadSpriteFrame(*spr, mem, spr->_cFrame);

    if (mem.loadedFrame == -1)
      continue;

    if (!mem.loadedIntoMemory)
      setOAMState(mem);

    setSpritePosAndScale(*spr, mem);
  }
}

void OAMManager::setOAMState(SpriteOAMMemory &mem) {
  for (u8 oamId : mem.oamEntries) {
    OAMEntry *oamEntry = &_oamEntries[oamId];
    auto *oamStart = (u16 *)((u8 *)_oamRam + oamId * 8);
    oamStart[0] = 0 << 13; // set 16 color mode
    oamStart[1] = 0;
    // set start tile and priority 0 and palette
    oamStart[2] = oamEntry->tileStart / 2 + (0 << 10) + (mem.palette << 12);
    // set size mode
    if (oamEntry->tileWidth == oamEntry->tileHeight) {
      switch (oamEntry->tileWidth) {
      case 2:
        oamStart[1] |= 1 << 14;
        break;
      case 4:
        oamStart[1] |= 2 << 14;
        break;
      case 8:
        oamStart[1] |= 3 << 14;
        break;
      default:
        break;
      }
    } else if (oamEntry->tileWidth > oamEntry->tileHeight) {
      oamStart[0] |= 1 << 14;
      switch (oamEntry->tileWidth) {
      case 4:
        if (oamEntry->tileHeight == 1)
          oamStart[1] |= 1 << 14;
        else
          oamStart[1] |= 2 << 14;
        break;
      case 8:
        oamStart[1] |= 3 << 14;
        break;
      default:
        break;
      }
    } else {
      oamStart[0] |= 2 << 14;
      switch (oamEntry->tileHeight) {
      case 4:
        if (oamEntry->tileWidth == 1)
          oamStart[1] |= 1 << 14;
        else
          oamStart[1] |= 2 << 14;
        break;
      case 8:
        oamStart[1] |= 3 << 14;
        break;
      default:
        break;
      }
    }
  }
  mem.loadedIntoMemory = true;
}

void OAMManager::setSpritePosAndScale(Engine::Sprite &spr,
                                      SpriteOAMMemory &mem) {
  u8 tileWidth, tileHeight;
  spr._texture->getSizeTiles(tileWidth, tileHeight);
  u8 oamW = (tileWidth + 7) / 8;
  u8 oamH = (tileHeight + 7) / 8;
  for (int oamY = 0; oamY < oamH; oamY++) {
    for (int oamX = 0; oamX < oamW; oamX++) {
      int oamId = mem.oamEntries[oamY * oamW + oamX];
      auto *oamStart = (u16 *)((u8 *)_oamRam + oamId * 8);
      bool useScale = (spr._scale_x != (1 << 8)) || (spr._scale_y != (1 << 8));
      if (useScale) {
        if (mem.oamScaleIdx == 0xff) {
          allocateOamScaleEntry(mem);
        }
      } else {
        if (mem.oamScaleIdx != 0xff) {
          freeOamScaleEntry(mem);
        }
      }

      oamStart[1] &= ~(0b1111 << 9);
      if (mem.oamScaleIdx != 0xff) {
        oamStart[0] |= 1 << 8; // set scale and rotation flag
        oamStart[0] |= 1 << 9; // set scale and rotation flag
        oamStart[1] |= mem.oamScaleIdx << 9;
        auto *oamScaleA = (u16 *)((u8 *)_oamRam + mem.oamScaleIdx * 0x20 + 0x6);
        auto *oamScaleB = (u16 *)((u8 *)_oamRam + mem.oamScaleIdx * 0x20 + 0xE);
        auto *oamScaleC =
            (u16 *)((u8 *)_oamRam + mem.oamScaleIdx * 0x20 + 0x16);
        auto *oamScaleD =
            (u16 *)((u8 *)_oamRam + mem.oamScaleIdx * 0x20 + 0x1E);
        *oamScaleA = (1 << 16) / spr._scale_x;
        *oamScaleB = 0;
        *oamScaleC = 0;
        *oamScaleD = (1 << 16) / spr._scale_y;
      } else {
        oamStart[0] &= ~(1 << 8);
        oamStart[0] &= ~(1 << 9);
      }
      oamStart[0] &= ~0xFF;
      s32 posX = spr._x + oamX * 64 * spr._scale_x;
      posX %= (512 << 8);
      if (posX < 0)
        posX = (512 << 8) + posX;
      s32 posY = spr._y + oamY * 64 * spr._scale_y;
      posY %= (256 << 8);
      if (posY < 0)
        posY = (256 << 8) + posY;
      oamStart[0] |= posY >> 8;
      oamStart[1] &= ~0x1FF;
      oamStart[1] |= posX >> 8;
      oamStart[2] &= ~(3 << 10);
      oamStart[2] |= (spr._layer & 0b11) << 10;
      // TODO: Disable oam if out of screen
    }
  }
}

void OAMManager::allocateOamScaleEntry(SpriteOAMMemory &mem) {
  for (int i = 0; i < 32; i++) {
    if (!_oamScaleEntryUsed[i]) {
      mem.oamScaleIdx = i;
      _oamScaleEntryUsed[i] = true;
      return;
    }
  }
}

void OAMManager::freeOamScaleEntry(SpriteOAMMemory &mem) {
  _oamScaleEntryUsed[mem.oamScaleIdx] = false;
  mem.oamScaleIdx = 0xff;
}

OAMManager OAMManagerSub(SPRITE_PALETTE_SUB, SPRITE_GFX_SUB, OAM_SUB);
} // namespace Engine
