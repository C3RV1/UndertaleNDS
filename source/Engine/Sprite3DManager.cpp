//
// Created by cervi on 25/08/2022.
//

#include "Engine/Sprite3DManager.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/dma.hpp"
#include <memory>
#include <utility>

namespace Engine {
int Sprite3DManager::loadSprite(std::shared_ptr<Sprite> res) {
  if (!res->_loaded)
    return -1;
  if (res->_allocated != NoAlloc)
    return -2;
  if (!res->_texture->_has3D) {
    std::string buffer = "Error loading spr #r" + res->_texture->_path +
                         "#x to 3D: Sprite doesn't have 3D chunk.";
    throw_(buffer);
  }

  Sprite3DMemory mem;
  mem.texture = res->_texture;
  mem.loadedFrame = -1;
  mem.loadedIntoMemory = false;

  _activeSpr.push_back(std::make_pair(res, mem));
  res->_allocated = Allocated3D;
  return 0;
}

void Sprite3DManager::freeSprite(std::shared_ptr<Sprite> spr) {
  if (spr->_allocated != Allocated3D)
    return;
  auto sprIdx = _activeSpr.begin();
  for (; sprIdx != _activeSpr.end(); sprIdx++) {
    if (sprIdx->first.lock() == spr)
      break;
  }
  if (sprIdx == _activeSpr.end())
    return;

  freeSpriteTexture(sprIdx->second);
  _activeSpr.erase(sprIdx);
  spr->_allocated = NoAlloc;
}

void Sprite3DManager::loadSpriteTexture(Sprite3DMemory &mem) {
  mem.texture->_loaded3DCount += 1;
  if (mem.texture->_loaded3DCount > 1) { // Already loaded to texture
    mem.loadedIntoMemory = true;
    return;
  }

  bool color8bit = mem.texture->_colors.size() > 15;
  u16 length, alignment, tileBytes;
  if (color8bit) {
    length = 16;
    alignment = 16;
    tileBytes = 64;
  } else {
    length = 1;
    alignment = 1;
    tileBytes = 32;
  }

  int res =
      paletteFreeZones.reserve(length, mem.texture->_paletteIdx, alignment);
  if (res != 0) {
    // no palette found
    std::string buffer = "Error loading spr #r" + mem.texture->_path +
                         "#x to 3D: No available palettes.";
    throw_(buffer);
  }

  u16 *paletteBase = &VRAM_E[16 * mem.texture->_paletteIdx + 1];
  dmaCopySafe(3, &mem.texture->_colors[0], paletteBase,
              mem.texture->_colors.size() * 2);

  int allocX = mem.texture->_3dChunk.tilesAllocX;
  int allocY = mem.texture->_3dChunk.tilesAllocY;
  mem.texture->_tileStart.resize(allocX * allocY);

  for (int tileY = 0; tileY < allocY; tileY++) {
    for (int tileX = 0; tileX < allocX; tileX++) {
      int tileIdx = tileY * allocX + tileX;
      auto &textureTile = mem.texture->_3dChunk.tiles[tileIdx];
      u8 tileWidth = textureTile.tileWidth;
      u8 tileHeight = textureTile.tileHeight;
      u16 neededBytes =
          tileWidth * tileHeight * mem.texture->_frameCount * tileBytes;
      if (tileFreeZones.reserve(neededBytes, mem.texture->_tileStart[tileIdx],
                                1) == 1) {
        std::string buffer = "Error loading spr #r" + mem.texture->_path +
                             "#x to 3D: Couldn't reserve tiles.";
        throw_(buffer);
      }

      u8 *tileRamStart = (u8 *)VRAM_B + mem.texture->_tileStart[tileIdx];
      dmaCopySafe(3, &textureTile.tileFrameData[0], tileRamStart, neededBytes);
    }
  }

  mem.loadedIntoMemory = true;
}

void Sprite3DManager::freeSpriteTexture(Sprite3DMemory &mem) {
  mem.texture->_loaded3DCount -= 1;
  if (mem.texture->_loaded3DCount > 0) // Texture used by another sprite
    return;

  u8 tileBytes = mem.texture->_colors.size() > 15 ? 64 : 32;

  int allocX = mem.texture->_3dChunk.tilesAllocX;
  int allocY = mem.texture->_3dChunk.tilesAllocY;

  for (int tileY = 0; tileY < allocY; tileY++) {
    for (int tileX = 0; tileX < allocX; tileX++) {
      int tileIdx = tileY * allocX + tileX;
      auto &textureTile = mem.texture->_3dChunk.tiles[tileIdx];
      u8 tileWidth = textureTile.tileWidth;
      u8 tileHeight = textureTile.tileHeight;
      u16 neededBytes =
          tileWidth * tileHeight * mem.texture->_frameCount * tileBytes;
      tileFreeZones.free(neededBytes, mem.texture->_tileStart[tileIdx]);
    }
  }

  mem.texture->_tileStart.clear();
}

ITCM_CODE
void Sprite3DManager::draw() {
  if (_activeSpr.empty())
    return;
  for (auto i = _activeSpr.begin(); i != _activeSpr.end();) {
    if (i->first.expired()) {
      freeSpriteTexture(i->second);
      i = _activeSpr.erase(i);
      continue;
    }

    std::shared_ptr<Sprite> spr = i->first.lock();
    Sprite3DMemory &mem = i->second;
    i++;

    spr->tick();

    if (!mem.loadedIntoMemory) {
      continue;
    }

    glColor(RGB15(31, 31, 31));
    if (spr->_opacity != 31)
      ensureAlphaBlend();
    glPolyFmt(POLY_ALPHA(spr->_opacity) | POLY_CULL_NONE);

    u8 tileFormat = mem.texture->_colors.size() > 15 ? 4 : 3;
    u8 tileBytes = mem.texture->_colors.size() > 15 ? 64 : 32;

    int allocX = mem.texture->_3dChunk.tilesAllocX;
    int allocY = mem.texture->_3dChunk.tilesAllocY;

    int tilePosY = 0;
    for (int tileY = 0; tileY < allocY; tileY++) {
      int tilePosX = 0;
      int tileH = 0;
      for (int tileX = 0; tileX < allocX; tileX++) {
        int tileIdx = tileY * allocX + tileX;
        auto &textureTile = mem.texture->_3dChunk.tiles[tileIdx];
        tileH = textureTile.tileHeight;

        s32 x = ((spr->_x - (1 << 4)) >> 8) + 1;
        x += (tilePosX * 8 * spr->_scale_x) >> 8;
        s32 x2 = x + ((textureTile.tileWidth * 8 * spr->_scale_x) >> 8);
        s32 w = textureTile.tileWidth * 8;
        s32 y = (((spr->_y - (1 << 4)) >> 8)) + 1;
        y += (tilePosY * 8 * spr->_scale_y) >> 8;
        s32 y2 = y + ((textureTile.tileHeight * 8 * spr->_scale_y) >> 8);
        s32 h = textureTile.tileHeight * 8;

        if (x > 256 || x2 < 0 || y > 192 || y2 < 0) {
          tilePosX += textureTile.tileWidth;
          continue;
        }

        int allocXFmt = 0;
        int allocYFmt = 0;
        for (; 1 << allocXFmt < textureTile.tileWidth; allocXFmt++)
          ;
        for (; 1 << allocYFmt < textureTile.tileHeight; allocYFmt++)
          ;

        MATRIX_CONTROL = GL_MODELVIEW;
        MATRIX_IDENTITY = 0;
        GFX_TEX_FORMAT = (allocXFmt << 20) + (allocYFmt << 23) +
                         (tileFormat << 26) + (1 << 29) +
                         (mem.texture->_tileStart[tileIdx] +
                          spr->_cFrame * textureTile.tileWidth *
                              textureTile.tileHeight * tileBytes) /
                             8;
        if (tileFormat == 4)
          GFX_PAL_FORMAT = mem.texture->_paletteIdx;
        else
          GFX_PAL_FORMAT = mem.texture->_paletteIdx * 2;
        GFX_BEGIN = GL_QUADS;
        GFX_TEX_COORD = 0;
        GFX_VERTEX16 = x + (y << 16);
        GFX_VERTEX16 = spr->_layer + mem.texture->_topDownOffset;
        GFX_TEX_COORD = h << (4 + 16);
        GFX_VERTEX_XY = x + (y2 << 16);
        GFX_TEX_COORD = (h << (4 + 16)) + (w << 4);
        GFX_VERTEX_XY = x2 + (y2 << 16);
        GFX_TEX_COORD = (w << 4);
        GFX_VERTEX_XY = x2 + (y << 16);
        GFX_END = 0;
        tilePosX += textureTile.tileWidth;
      }
      tilePosY += tileH;
    }
  }
}

ITCM_CODE
void Sprite3DManager::ensureAlphaBlend() {
  // Set 3d alpha blend
  REG_BLDCNT = 1 | (1 << 6) | 1 << 11; // 1st BG0 + alpha + 2nd BG3
}

ITCM_CODE
void Sprite3DManager::updateTextures() {
  bool setBank = false;
  if (_activeSpr.empty())
    return;
  for (auto &i : _activeSpr) {
    if (i.first.expired())
      continue; // Will get freed on draw.

    Sprite3DMemory &mem = i.second;
    if (!mem.loadedIntoMemory) {
#ifdef DEBUG_3D
      nocashMessage("Loading sprite");
#endif
      if (!setBank) {
        vramSetBankB(VRAM_B_LCD);
        vramSetBankE(VRAM_E_LCD);
        setBank = true;
      }
      loadSpriteTexture(mem);
    }
  }
  if (setBank) {
    vramSetBankB(VRAM_B_TEXTURE_SLOT0);
    vramSetBankE(VRAM_E_TEX_PALETTE);
  }
}

Sprite3DManager main3dSpr;
} // namespace Engine
