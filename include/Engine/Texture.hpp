#ifndef UNDERTALE_TEXTURE_HPP
#define UNDERTALE_TEXTURE_HPP

#include <cstdio>
#define ARM9
#include <nds.h>
#include <vector>
#include "Formats/CSPR.hpp"

namespace Engine {

    class Texture {
    public:
        bool loadPath(const std::string& path);
        void loadCSPR(FILE* f);
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
        void loadOam(FILE* f);
        void load3D(FILE* f);
        void free_();
        friend class OAMManager;
        friend class Sprite3DManager;
        friend class Sprite;
        bool _loaded = false;
        bool _hasOam = false;
        bool _has3D = false;

        // Header chunk
        u16 _width = 0, _height = 0; // don't know if these will be used
        u16 _topDownOffset = 0;
        u8 _frameCount = 0;

        // Color chunk
        std::vector<u16> _colors;

        // Animations chunk
        u8 _animationCount = 0;
        std::vector<CSPRAnimation> _animations;

        // 3D
        u8 _loaded3DCount = 0;
        std::vector<u16> _tileStart;
        u16 _paletteIdx = 0;

        CSPR_OAM _oamChunk;
        CSPR_3D _3dChunk;

        std::string _path;
    };
}

#endif //UNDERTALE_TEXTURE_HPP
