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
struct Sprite3DMemory {
  std::shared_ptr<Texture> texture;

  int loadedFrame = -1;
  bool loadedIntoMemory = false;
};

class Sprite3DManager {
public:
  Sprite3DManager()
      : tileFreeZones(0, 65536 - 8, "3D_TILES"),
        paletteFreeZones(0, 1024, "3D_PALETTE") {}

  void draw();
  void updateTextures();

private:
  friend void spriteSetShown(std::shared_ptr<Sprite> spr, bool shown);

  int loadSprite(std::shared_ptr<Sprite> spr);
  void freeSprite(std::shared_ptr<Sprite> spr);

  void ensureAlphaBlend();

  void loadSpriteTexture(Sprite3DMemory &mem);
  void freeSpriteTexture(Sprite3DMemory &mem);

  FreeZoneManager tileFreeZones;
  FreeZoneManager paletteFreeZones;

  std::vector<std::pair<std::weak_ptr<Sprite>, Sprite3DMemory>> _activeSpr;
};

extern Sprite3DManager main3dSpr;
} // namespace Engine

#endif // UNDERTALE_SPRITE_3D_MANAGER_HPP
