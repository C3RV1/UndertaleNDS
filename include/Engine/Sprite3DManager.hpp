//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_SPRITE_3D_MANAGER_HPP
#define UNDERTALE_SPRITE_3D_MANAGER_HPP

#include "Engine/FreeZoneManager.hpp"
#include "Sprite.hpp"
#include "Texture.hpp"
#include <memory>
#include <nds.h>
#include <vector>

namespace Engine {
class Sprite3DTexture {
public:
  Sprite3DTexture(FreeZoneManager &tileReserve, FreeZoneManager &paletteReserve,
                  const Texture& texture);
  ~Sprite3DTexture();

  inline u16 getTile(size_t i) const {
    return _tiles[i].first;
  }

  inline size_t getTileCount() const {
    return _tiles.size();
  }

  inline u16 getPaletteStart() const {
    return _palette.first;
  }

private:
  FreeZoneManager& _tileReserve;
  FreeZoneManager& _paletteReserve;
  std::vector<std::pair<u16, u16>> _tiles;
  std::pair<u16, u16> _palette;
};

struct Sprite3DMemory {
  std::shared_ptr<Sprite3DTexture> texture = nullptr;
  int loadedFrame = -1;
};

class Sprite3DManager {
public:
  Sprite3DManager()
      : tileFreeZones(0, 65536 - 8, "3D_TILES"),
        paletteFreeZones(0, 1024, "3D_PALETTE") {}

  void sendToGeometryEngine();
  void updateTextures();

private:
  friend void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown);

  int loadSprite(std::shared_ptr<Sprite> spr);
  void freeSprite(std::shared_ptr<Sprite> spr);

  void ensureAlphaBlend();

  std::shared_ptr<Sprite3DTexture> loadSpriteTexture(const Texture &texture, bool &setBank);
  void freeSpriteTexture(Sprite3DMemory &mem);

  FreeZoneManager tileFreeZones;
  FreeZoneManager paletteFreeZones;

  std::vector<std::pair<std::weak_ptr<Sprite>, Sprite3DMemory>> _activeSpr;
};

extern Sprite3DManager main3dSpr;
} // namespace Engine

#endif // UNDERTALE_SPRITE_3D_MANAGER_HPP
