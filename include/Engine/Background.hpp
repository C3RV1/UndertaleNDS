//
// Created by cervi on 19/08/2022.
//

#ifndef LAYTON_BACKGROUND_HPP
#define LAYTON_BACKGROUND_HPP

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define ARM9
#include <nds.h>

namespace Engine {
    struct Tile {
        uint8_t* tileData = nullptr;
    };

    class Background {
    public:
        int loadCBGF(FILE* f);
        bool getLoaded() const { return loaded; }
        bool getColor8bit() const { return color8bit; }
        uint8_t getColorCount() const { return colorCount; }
        uint16_t* getColors() const { return colors; }
        uint16_t getTileCount() const { return tileCount; }
        uint8_t* getTiles() const { return tiles; }
        void getSize(uint16_t& l_width, uint16_t& l_height) const {
            l_width = width;
            l_height = height;
        }
        uint16_t* getMap() const { return map; }
        void free_();
        ~Background() { free_(); }
    private:
        bool loaded = false;
        bool color8bit = false;
        uint8_t colorCount = 0;
        uint16_t* colors = nullptr;
        uint16_t tileCount = 0;
        uint8_t* tiles = nullptr;
        uint16_t width = 0, height = 0;
        uint16_t* map = nullptr;
    };
}

#endif //LAYTON_BACKGROUND_HPP
