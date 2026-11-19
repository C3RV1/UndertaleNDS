//
// Created by cervi on 25/08/2022.
//

#include "Engine/Sprite3DManager.hpp"
#include "DEBUG_FLAGS.hpp"
#include "Engine/Engine.hpp"
#include "Engine/FreeZoneManager.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/dma.hpp"
#include <memory>
#include <utility>

namespace Engine {
int Sprite3DManager::loadSprite(std::shared_ptr<Sprite> res) {
  if (!res->_loaded)
    return -1;
  if (!res->_texture->getHas3D())
    throw_("Error loading spr #r" + res->_texture->getPath() +
           "#x to 3D: Sprite doesn't have 3D chunk.");

  for (auto &entry : _activeSpr) {
    if (entry.first.lock() == res)
      return -2;
  }
  
  Sprite3DMemory mem;
  mem.texture = nullptr;
  mem.loadedFrame = -1;

  _activeSpr.push_back(std::make_pair(res, mem));
  return 0;
}

void Sprite3DManager::freeSprite(std::shared_ptr<Sprite> spr) {
  auto sprIdx = _activeSpr.begin();
  for (; sprIdx != _activeSpr.end(); sprIdx++) {
    if (sprIdx->first.lock() == spr)
      break;
  }
  if (sprIdx == _activeSpr.end())
    return;

  _activeSpr.erase(sprIdx);
}

ITCM_CODE
void Sprite3DManager::sendToGeometryEngine() {
  for (auto &i : _activeSpr) {
    if (i.first.expired()) {
      continue;
    }

    Sprite& spr = *(i.first.lock());
    Texture& texture = *spr._texture;
    Sprite3DMemory &mem = i.second;

    spr.tick();

    if (mem.texture == nullptr) {
      continue;
    }

    Sprite3DTexture &texture3D = *mem.texture;

    glColor(RGB15(31, 31, 31));
    if (spr._opacity != 31)
      ensureAlphaBlend();
    glPolyFmt(POLY_ALPHA(spr._opacity) | POLY_CULL_NONE);

    u8 tileFormat = texture.getColors().size() > 15 ? 4 : 3;
    u8 tileBytes = texture.getColors().size() > 15 ? 64 : 32;

    int allocX = texture.get3DChunk().tilesAllocX;
    int allocY = texture.get3DChunk().tilesAllocY;

    s32 y = spr._y;
    for (int tileY = 0; tileY < allocY; tileY++) {
      s32 x = spr._x;
      s32 y2 = 0;
      for (int tileX = 0; tileX < allocX; tileX++) {
        int tileIdx = tileY * allocX + tileX;
        auto &textureTile = texture.get3DChunk().tiles[tileIdx];

        s32 w = textureTile.tileWidth * 8;
        s32 x2 = x + w * spr._scale_x;
        
        s32 h = textureTile.tileHeight * 8;
        y2 = y + h * spr._scale_y;

        s32 px = x >> 8, py = y >> 8, px2 = x2 >> 8, py2 = y2 >> 8;
        if (px > 256 || px2 < 0 || py > 192 || py2 < 0) {
          x = x2;
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
                         (texture3D.getTile(tileIdx) +
                          spr._cFrame * textureTile.tileWidth *
                              textureTile.tileHeight * tileBytes) /
                             8;
        if (tileFormat == 4)
          GFX_PAL_FORMAT = texture3D.getPaletteStart();
        else
          GFX_PAL_FORMAT = texture3D.getPaletteStart() * 2;
        GFX_BEGIN = GL_QUADS;
        GFX_TEX_COORD = 0;
        GFX_VERTEX16 = px + (py << 16);
        GFX_VERTEX16 = spr._layer + texture.getTopDownOffset();
        GFX_TEX_COORD = h << (4 + 16);
        GFX_VERTEX_XY = px + (py2 << 16);
        GFX_TEX_COORD = (h << (4 + 16)) + (w << 4);
        GFX_VERTEX_XY = px2 + (py2 << 16);
        GFX_TEX_COORD = (w << 4);
        GFX_VERTEX_XY = px2 + (py << 16);
        GFX_END = 0;
        x = x2;
      }
      y = y2;
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
  for (auto i = _activeSpr.begin(); i != _activeSpr.end();) {
    if (i->first.expired()) {
      i = _activeSpr.erase(i);
      continue; // Will get freed on draw.
    }

    Texture &texture = *i->first.lock()->_texture;
    Sprite3DMemory &mem = i->second;
    i++;
    
    if (mem.texture == nullptr) {
      debug_3d("Loading sprite");
      mem.texture = loadSpriteTexture(texture, setBank);
    }
  }
  if (setBank) {
    vramSetBankB(VRAM_B_TEXTURE_SLOT0);
    vramSetBankE(VRAM_E_TEX_PALETTE);
  }
}

std::shared_ptr<Sprite3DTexture> Sprite3DManager::loadSpriteTexture(const Texture &texture,
                                                                    bool &setBank) {
  for (auto& entry : _activeSpr) {
    if (entry.first.expired())
      continue;
    
    Sprite &spr = *entry.first.lock();
    Sprite3DMemory &mem = entry.second;
    
    if (spr._texture.get() == &texture && mem.texture != nullptr) {
      return mem.texture;
    }
  }

  if (!setBank) {
    vramSetBankB(VRAM_B_LCD);
    vramSetBankE(VRAM_E_LCD);
    setBank = true;
  }
  return std::make_shared<Sprite3DTexture>(tileFreeZones, paletteFreeZones, texture);
}

Sprite3DTexture::Sprite3DTexture(FreeZoneManager& tileReserve,
                                 FreeZoneManager& paletteReserve,
                                 const Texture& texture)
    : _tileReserve(tileReserve), _paletteReserve(paletteReserve) {
  debug_3d("Loading texture 3d: " + texture.getPath());
  bool color8bit = texture.getColors().size() > 15;
  u16 length, alignment, tileBytes;
  if (color8bit) {
    length = 16;
    alignment = 16;
    tileBytes = 64;
  } else {
    // FIXME: Is this right?
    length = 1;
    alignment = 1;
    tileBytes = 32;
  }

  _palette.second = length;
  int res = paletteReserve.reserve(length, _palette.first, alignment);
  if (res != 0) {
    // no palette found
    throw_("Error loading spr #r" + texture.getPath() +
           "#x to 3D: No available palettes.");
  }

  u16 *paletteBase = &VRAM_E[16 * _palette.first + 1];
  dmaCopySafe(3, &texture.getColors()[0], paletteBase,
              texture.getColors().size() * 2);

  int allocX = texture.get3DChunk().tilesAllocX;
  int allocY = texture.get3DChunk().tilesAllocY;
  _tiles.resize(allocX * allocY);

  for (int tileY = 0; tileY < allocY; tileY++) {
    for (int tileX = 0; tileX < allocX; tileX++) {
      int tileIdx = tileY * allocX + tileX;
      auto &textureTile = texture.get3DChunk().tiles[tileIdx];
      u8 tileWidth = textureTile.tileWidth;
      u8 tileHeight = textureTile.tileHeight;
      u16 neededBytes =
          tileWidth * tileHeight * texture.getFrameCount() * tileBytes;
        
      _tiles[tileIdx].second = neededBytes;
      if (tileReserve.reserve(neededBytes, _tiles[tileIdx].first,
                                1) == 1) {
        throw_("Error loading spr #r" + texture.getPath() +
               "#x to 3D: Couldn't reserve tiles.");
      }

      u8 *tileRamStart = (u8 *)VRAM_B + _tiles[tileIdx].first;
      dmaCopySafe(3, &textureTile.tileFrameData[0], tileRamStart, neededBytes);
    }
  }
}

Sprite3DTexture::~Sprite3DTexture() {
  _paletteReserve.free(_palette.second, _palette.first);
  for (std::pair<u16, u16> entry : _tiles) {
    _tileReserve.free(entry.second, entry.first);
  }
}

Sprite3DManager main3dSpr;
} // namespace Engine
