#ifndef UNDERTALE_TEXTURE_HPP
#define UNDERTALE_TEXTURE_HPP

#include <cstdio>
#define ARM9
#include <nds.h>
#include "Formats/CSPR.hpp"

namespace Engine {
    class Texture {
    public:
        bool loadPath(const char* path);
        int loadCSPR(FILE* f);
        bool getLoaded() const { return _loaded; }
        void getSizeTiles(u8& tileWidth, u8& tileHeight) const {
            tileWidth = (_width + 7) / 8;
            tileHeight = (_height + 7) / 8;
        }
        u16 getWidth() const {
            return _width;
        }
        u16 getHeight() const {
            return _height;
        }
        void getSize(u16& width, u16& height) const {
            width = _width;
            height = _height;
        }
        ~Texture() { free_(); }
    private:
        void free_();
        friend class OAMManager;
        friend class Sprite3DManager;
        friend class Sprite;
        bool _loaded = false;
        u8 _colorCount = 0;
        u16* _colors = nullptr;
        u16 _width = 0, _height = 0;
        u8 _frameCount = 0;
        u8 _animationCount = 0;
        u16 _topDownOffset = 0;
        CSPRAnimation* _animations = nullptr;
        u8* _tiles = nullptr;

        // 3D
        u8 _loaded3DCount = 0;
        u16 * _tileStart = nullptr;
        u16 _paletteIdx = 0;
        bool _color8bit = false;

        friend class Sprite3DManager;
    };
}

#endif //UNDERTALE_TEXTURE_HPP
