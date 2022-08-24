#include "Engine/Sprite.hpp"

namespace Engine {
    int Sprite::loadCSPR(FILE *f) {
        free_();
        char header[4];
        uint32_t fileSize;
        uint32_t version;
        fread(header, 4, 1, f);

        const char expectedChar[4] = {'C', 'S', 'P', 'R'};
        if (memcmp(header, expectedChar, 4) != 0) {
            return 1;
        }

        fread(&fileSize, 4, 1, f);
        uint32_t pos = ftell(f);
        fseek(f, 0, SEEK_END);
        uint32_t size = ftell(f);
        fseek(f, pos, SEEK_SET);

        if (fileSize != size) {
            return 2;
        }

        fread(&version, 4, 1, f);
        if (version != 1) {
            return 3;
        }

        fread(&tileWidth, 1, 1, f);
        fread(&tileHeight, 1, 1, f);

        fread(&colorCount, 1, 1, f);
        colors = (uint16_t*) malloc(colorCount * 2);
        fread(colors, 2, colorCount, f);

        fread(&frameCount, 1, 1, f);
        uint16_t tileCount = tileWidth * tileHeight;
        tiles = (uint8_t*) malloc(8 * 8 * tileCount * frameCount);
        fread(tiles, 8 * 8 * tileCount * frameCount, 1, f);

        loaded = true;
        return 0;
    }

    void Sprite::free_() {
        if (!loaded)
            return;
        loaded = false;
        free(colors);
        colors = nullptr;
        free(tiles);
        tiles = nullptr;
    }
}