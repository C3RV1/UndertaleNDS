//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_FONT_HPP
#define UNDERTALE_FONT_HPP

#include <cstdio>
#include <string>

#include "Formats/CFNT.hpp"
#include <nds.h>

namespace Engine {
class Font {
public:
  bool loadPath(const std::string &path);
  void loadCFNT(FILE *f);
  bool getLoaded() const { return _loaded; }
  u8 getGlyphWidth(u8 glyph);
  ~Font() { free_(); }

private:
  u8 *getGlyphMap() { return _glyphMap.glyphMap; }
  const CFNTGlyph *getGlyph(int glyphIdx) const {
    return &_glyphs.glyphs[glyphIdx - 1];
  }
  friend class TextBGManager;
  bool _loaded = false;
  CFNTGlyphs _glyphs;
  CFNTMap _glyphMap;
  void free_();

  std::string _path;
};

} // namespace Engine

#endif // UNDERTALE_FONT_HPP
