#include "Engine/Texture.hpp"
#include "Formats/utils.hpp"

namespace Engine {
    bool Texture::loadPath(const char *path) {
        char pathFull[100];
        char buffer[100];

        sprintf(pathFull, "nitro:/spr/%s.cspr", path);

        FILE* f = fopen(pathFull, "rb");
        if (!f) {
            sprintf(buffer, "Error opening spr %s", path);
            nocashMessage(buffer);
            return false;
        }

        int loadRes = loadCSPR(f);

        fclose(f);

        if (loadRes != 0) {
            sprintf(buffer, "Error loading spr %s: %d", path, loadRes);
            nocashMessage(buffer);
            return false;
        }

        return true;
    }

    int Texture::loadCSPR(FILE *f) {
        free_();
        char header[4];
        u32 fileSize;
        u32 version;
        fread(header, 4, 1, f);

        const char expectedChar[4] = {'C', 'S', 'P', 'R'};
        if (memcmp(header, expectedChar, 4) != 0) {
            return 1;
        }

        fread(&fileSize, 4, 1, f);
        u32 pos = ftell(f);
        fseek(f, 0, SEEK_END);
        u32 size = ftell(f);
        fseek(f, pos, SEEK_SET);

        if (fileSize != size) {
            return 2;
        }

        fread(&version, 4, 1, f);
        if (version != 4) {
            return 3;
        }

        fread(&width, 2, 1, f);
        fread(&height, 2, 1, f);
        u16 tileWidth = (width + 7) / 8, tileHeight = (height + 7) / 8;

        fread(&colorCount, 1, 1, f);
        colors = new u16[colorCount];
        fread(colors, 2, colorCount, f);

        fread(&frameCount, 1, 1, f);
        u16 tileCount = tileWidth * tileHeight;
        tiles = new u8[64 * tileCount * frameCount];
        fread(tiles, 8 * 8 * tileCount * frameCount, 1, f);

        fread(&animationCount, 1, 1, f);
        animations = new CSPRAnimation[animationCount];
        for (int i = 0; i < animationCount; i++) {
            int nameLen = strlen_file(f, 0);
            animations[i].name = new char[nameLen + 1];
            fread(animations[i].name, nameLen + 1, 1, f);
            fread(&animations[i].frameCount, 1, 1, f);
            animations[i].frames = new CSPRAnimFrame[animations[i].frameCount];
            if (animations[i].frameCount == 0) {
                // should free on error?
                return 4;
            }
            for (int j = 0; j < animations[i].frameCount; j++) {
                fread(&animations[i].frames[j].frame, 1, 1, f);
                fread(&animations[i].frames[j].duration, 2, 1, f);
                fread(&animations[i].frames[j].drawOffX, 1, 1, f);
                fread(&animations[i].frames[j].drawOffY, 1, 1, f);
            }
        }

        loaded = true;
        return 0;
    }

    void Texture::free_() {
        if (!loaded)
            return;
        loaded = false;
        delete[] colors;
        colors = nullptr;
        delete[] tiles;
        tiles = nullptr;
        for (int i = 0; i < animationCount; i++) {
            delete[] animations[i].name;
            animations[i].name = nullptr;
            delete[] animations[i].frames;
            animations[i].frames = nullptr;
        }
        delete[] animations;
        animations = nullptr;
    }
}