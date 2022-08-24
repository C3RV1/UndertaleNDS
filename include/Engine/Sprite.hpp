#ifndef LAYTON_SPRITE_HPP
#define LAYTON_SPRITE_HPP

#include <stdlib.h>
#include <stdio.h>
#define ARM9
#include <nds.h>
#include "Background.hpp"

namespace Engine {
    class Sprite {
    public:
        int loadCSPR(FILE* f);
        bool getLoaded() const { return loaded; }
        int getColorCount() const { return colorCount; }
        uint16_t* getColors() const { return colors; }
        uint8_t getFrameCount() const { return frameCount; }
        void getSizeTiles(uint8_t& tileWidth_, uint8_t& tileHeight_) const {
            tileWidth_ = tileWidth;
            tileHeight_ = tileHeight;
        }
        uint8_t* getTiles() const { return tiles; }
        void free_();
        ~Sprite() { free_(); }
    private:
        bool loaded = false;
        uint8_t colorCount = 0;
        uint16_t* colors = nullptr;
        uint8_t tileWidth = 0, tileHeight = 0; // each tile is 8x8 (max of 64x64)
        uint8_t frameCount = 0;
        uint8_t* tiles = nullptr;
    };
}

#endif //LAYTON_SPRITE_HPP
