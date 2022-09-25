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
        int loadCSPR(FILE* f);
        bool getLoaded() const { return loaded; }
        int getColorCount() const { return colorCount; }
        uint16_t* getColors() const { return colors; }
        uint8_t getFrameCount() const { return frameCount; }
        uint8_t getAnimCount() const { return animationCount; }
        CSPRAnimation* getAnims() const { return animations;  }
        void getSizeTiles(uint8_t& tileWidth_, uint8_t& tileHeight_) const {
            tileWidth_ = (width + 7) / 8;
            tileHeight_ = (height + 7) / 8;
        }
        uint16_t getWidth() const {
            return width;
        }
        uint16_t getHeight() const {
            return height;
        }
        void getSize(uint16_t& width_, uint16_t& height_) const {
            width_ = width;
            height_ = height;
        }
        uint8_t* getTiles() const { return tiles; }
        void free_();
        ~Texture() { free_(); }
    private:
        bool loaded = false;
        uint8_t colorCount = 0;
        uint16_t* colors = nullptr;
        uint16_t width = 0, height = 0;
        uint8_t frameCount = 0;
        uint8_t animationCount = 0;
        CSPRAnimation* animations = nullptr;
        uint8_t* tiles = nullptr;
    };
}

#endif //UNDERTALE_TEXTURE_HPP
