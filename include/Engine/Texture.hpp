#ifndef UNDERTALE_TEXTURE_HPP
#define UNDERTALE_TEXTURE_HPP

#include "Formats/CSPR.hpp"
#include <cstdio>
#include <map>
#include <memory>
#include <nds.h>
#include <vector>

namespace Engine {

class Texture {
public:
  Texture(const std::string &path);
  bool loadPath(const std::string &path);
  void loadCSPR(FILE *f);
  bool getLoaded() const { return _loaded; }
  void getSizeTiles(u8 &tileWidth, u8 &tileHeight) const {
    tileWidth = (_width + 7) / 8;
    tileHeight = (_height + 7) / 8;
  }
  u16 getWidth() const { return _width; }
  u16 getHeight() const { return _height; }
  void getSize(u16 &width, u16 &height) const {
    width = _width;
    height = _height;
  }
  ~Texture() { free_(); }

  inline const std::vector<u16>& getColors() const {
    return _colors;
  }

  inline const CSPR_3D& get3DChunk() const {
    return _3dChunk;
  }
  
  inline const CSPR_OAM& getOAMChunk() const {
    return _oamChunk;
  }

  inline u8 getFrameCount() const {
    return _frameCount;
  }

  inline const std::string& getPath() const {
    return _path;
  }

  inline bool getHas3D() const {
    return _has3D;
  }

  inline bool getHasOAM() const {
    return _hasOam;
  }

  inline u16 getTopDownOffset() const {
    return _topDownOffset;
  }

  inline const std::vector<CSPRAnimation>& getAnimations() const {
    return _animations;
  }

private:
  void loadOam(FILE *f);
  void load3D(FILE *f);
  void free_();
  bool _loaded = false;
  bool _hasOam = false;
  bool _has3D = false;

  // Header chunk
  u16 _width = 0, _height = 0;
  u16 _topDownOffset = 0;
  u8 _frameCount = 0;

  // Color chunk
  std::vector<u16> _colors;

  // Animations chunk
  std::vector<CSPRAnimation> _animations;

  CSPR_OAM _oamChunk;
  CSPR_3D _3dChunk;

  std::string _path;
};

class TextureManager {
public:
  std::shared_ptr<Engine::Texture> loadTexture(const std::string &path,
                                               bool ensureReuse = false);
  void clearEnsureReuse(std::string path);

private:
  std::map<std::string, std::weak_ptr<Engine::Texture>> textures;
  std::map<std::string, std::shared_ptr<Engine::Texture>> ensureReuseTextures;
};

extern TextureManager textureManager;
} // namespace Engine

#endif // UNDERTALE_TEXTURE_HPP
