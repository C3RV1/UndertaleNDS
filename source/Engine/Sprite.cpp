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
        if (version != 2) {
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

        fread(&animationCount, 1, 1, f);
        animations = (CSPRAnimation*) malloc(sizeof(CSPRAnimation) * animationCount);
        for (int i = 0; i < animationCount; i++) {
            int nameLen = strlen_file(f);
            animations[i].name = (char*)malloc(nameLen + 1);
            fread(animations[i].name, nameLen + 1, 1, f);
            fread(&animations[i].frameCount, 1, 1, f);
            animations[i].frames = (CSPRAnimFrame*) malloc(sizeof(CSPRAnimFrame) * animations[i].frameCount);
            if (animations[i].frameCount == 0) {
                // should free on error?
                return 4;
            }
            for (int j = 0; j < animations[i].frameCount; j++) {
                fread(&animations[i].frames[j].frame, 1, 1, f);
                fread(&animations[i].frames[j].duration, 2, 1, f);
            }
        }

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
        for (int i = 0; i < animationCount; i++) {
            free(animations[i].name);
            free(animations[i].frames);
        }
        free(animations);
        animations = nullptr;
    }

    int Sprite::nameToAnimId(const char *animName) {
        if (!loaded)
            return -1;
        for (int i = 0; i < animationCount; i++) {
            if (strcmp(animName, animations[i].name) == 0) {
                return i;
            }
        }
        return -1;
    }
}