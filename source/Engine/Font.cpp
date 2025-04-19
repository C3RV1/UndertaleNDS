//
// Created by cervi on 23/08/2022.
//

#include "Engine/Font.hpp"
#include "Engine/Engine.hpp"

namespace Engine {
bool Font::loadPath(const std::string &path) {
  std::string pathFull = "nitro:/fnt/" + path + ".cfnt";
  _path = path;

  FILE *f = fopen(pathFull.c_str(), "rb");
  if (!f) {
    std::string buffer = "Error opening font #r" + _path;
    throw_(buffer);
  }

  loadCFNT(f);

  fclose(f);

  return true;
}

void Font::loadCFNT(FILE *f) {
  free_();
  char header[4];
  u32 fileSize;
  u32 version;
  fread(header, 4, 1, f);

  const char expectedChar[4] = {'C', 'F', 'N', 'T'};
  if (memcmp(header, expectedChar, 4) != 0) {
    std::string buffer =
        "Error loading font #r" + _path + "#x: Invalid header.";
    throw_(buffer);
  }

  fread(&fileSize, 4, 1, f);
  u32 pos = ftell(f);
  fseek(f, 0, SEEK_END);
  u32 size = ftell(f);
  fseek(f, pos, SEEK_SET);

  if (fileSize != size) {
    std::string buffer =
        "Error loading font #r" + _path +
        "#x: File size doesn't match (expected: " + std::to_string(fileSize) +
        ", actual: " + std::to_string(size) + ")";
    throw_(buffer);
  }

  fread(&version, 4, 1, f);
  if (version != CFNTHeader::version) {
    std::string buffer =
        "Error loading spr #r" + _path +
        "#x: Invalid version (expected: 1, actual: " + std::to_string(version) +
        ")";
    throw_(buffer);
  }

  fread(&_glyphs.lineHeight, 1, 1, f);
  u8 glyphCount;
  fread(&glyphCount, 1, 1, f);
  _glyphs.glyphs.resize(glyphCount);

  for (auto &glyph : _glyphs.glyphs) {
    fread(&glyph.width, 1, 1, f);
    fread(&glyph.height, 1, 1, f);
    fread(&glyph.shift, 1, 1, f);
    fread(&glyph.offset, 1, 1, f);
    u16 dataBytes = ((glyph.width * glyph.height + 7) / 8);
    glyph.glyphData.resize(dataBytes);
    fread(&glyph.glyphData[0], dataBytes, 1, f);
  }

  fread(_glyphMap.glyphMap, 1, 256, f);

  _loaded = true;
}

u8 Font::getGlyphWidth(u8 glyph) {
  u8 glyphIdx = getGlyphMap()[glyph];
  if (glyphIdx == 0)
    return 0;
  const CFNTGlyph *glyphObj = getGlyph(glyphIdx);
  return glyphObj->shift;
}

void Font::free_() {
  if (!_loaded)
    return;
  _loaded = false;
}

std::shared_ptr<Engine::Font> FontManager::loadFont(const std::string &path) {
  auto fontKV = fonts.find(path);
  if (fontKV != fonts.end()) {
    if (!fontKV->second.expired()) {
#ifdef DEBUG_FONTS
      std::string msg = "Found font " + path + " in bank: reusing.";
      nocashMessage(msg.c_str());
#endif
      return fontKV->second.lock();
    }
#ifdef DEBUG_FONTS
    else {
      std::string msg = "Found font " + path + " in bank: expired.";
      nocashMessage(msg.c_str());
    }
#endif
    fonts.erase(fontKV);
  }
#ifdef DEBUG_FONTS
  else {
    std::string msg = "Not found font " + path + " in bank.";
    nocashMessage(msg.c_str());
  }
#endif

  auto font = std::make_shared<Font>();
  font->loadPath(path);
  fonts.insert(std::make_pair(path, font));
  return font;
}

FontManager fontManager;

} // namespace Engine
