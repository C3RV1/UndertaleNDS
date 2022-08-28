//
// Created by cervi on 19/08/2022.
//
#include "Engine/Background.hpp"

namespace Engine {
    int Background::loadCBGF(FILE *f) {
        free_();
        char header[4];
        uint32_t fileSize;
        uint32_t version;
        uint8_t fileFormat;
        fread(header, 4, 1, f);
        uint32_t pos = ftell(f);
        fseek(f, 0, SEEK_END);
        uint32_t size = ftell(f);
        fseek(f, pos, SEEK_SET);

        const char expectedChar[4] = {'C', 'B', 'G', 'F'};
        if (memcmp(header, expectedChar, 4) != 0) {
            char buffer[100];
            sprintf(buffer, "Header %x %x %x %x size %x", header[0], header[1], header[2], header[3], size);
            nocashMessage(buffer);
            return 1;
        }

        fread(&fileSize, 4, 1, f);

        if (fileSize != size) {
            return 2;
        }

        fread(&version, 4, 1, f);
        if (version != 1) {
            return 3;
        }

        fread(&fileFormat, 1, 1, f);
        color8bit = fileFormat & 1;

        fread(&colorCount, 1, 1, f);
        if ((colorCount > 249 && color8bit) || (colorCount > 15 && !color8bit)) {
            return 4;
        }

        colors = (uint16_t*) malloc(2 * colorCount);
        fread(colors, 2, colorCount, f);

        fread(&tileCount, 2, 1, f);
        if (tileCount > 1024) {
            return 5;
        }

        uint32_t tileDataSize = 32;
        if (color8bit)
            tileDataSize = 64;

        tiles = (uint8_t *) malloc(sizeof(uint8_t*) * tileCount * tileDataSize);
        fread(tiles, tileDataSize, tileCount, f);

        fread(&width, 1, 1, f);
        fread(&height, 1, 1, f);

        map = (uint16_t*) malloc(2 * width * height);
        fread(map, 2, width * height, f);

        loaded = true;
        return 0;
    }

    void Background::free_() {
        if (!loaded)
            return;
        loaded = false;
        free(colors);
        colors = nullptr;
        free(tiles);
        tiles = nullptr;
        free(map);
        map = nullptr;
    }
}