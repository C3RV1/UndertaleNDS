#ifndef UNDERTALE_TEXTURE_HPP
#define UNDERTALE_TEXTURE_HPP

#include <stdlib.h>
#include <stdio.h>
#define ARM9
#include <nds.h>
#include "Formats/CSPR.hpp"

namespace Engine {
    class Texture {
    public:
        bool loadPath(const char* path);
        int loadCSPR(FILE* f);
        bool getLoaded() const { return loaded; }
        int getColorCount() const { return colorCount; }
        u16* getColors() const { return colors; }
        u8 getFrameCount() const { return frameCount; }
        u8 getAnimCount() const { return animationCount; }
        CSPRAnimation* getAnimations() const { return animations;  }
        void getSizeTiles(u8& tileWidth_, u8& tileHeight_) const {
            tileWidth_ = (width + 7) / 8;
            tileHeight_ = (height + 7) / 8;
        }
        u16 getWidth() const {
            return width;
        }
        u16 getHeight() const {
            return height;
        }
        void getSize(u16& width_, u16& height_) const {
            width_ = width;
            height_ = height;
        }
        u8* getTiles() const { return tiles; }
        void free_();
        ~Texture() { free_(); }
    private:
        bool loaded = false;
        u8 colorCount = 0;
        u16* colors = nullptr;
        u16 width = 0, height = 0;
        u8 frameCount = 0;
        u8 animationCount = 0;
        u16 topDownOffset = 0;
        CSPRAnimation* animations = nullptr;
        u8* tiles = nullptr;

        // 3D
        u8 loaded3DCount = 0;
        u16 * tileStart = nullptr;
        u16 allocX = 0, allocY = 0;
        u8 paletteIdx;

        friend class Sprite3DManager;
    };
}

#endif //UNDERTALE_TEXTURE_HPP
