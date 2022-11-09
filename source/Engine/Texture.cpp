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

        fread(&_width, 2, 1, f);
        fread(&_height, 2, 1, f);
        fread(&_topDownOffset, 2, 1, f);
        u16 tileWidth = (_width + 7) / 8, tileHeight = (_height + 7) / 8;

        fread(&_colorCount, 1, 1, f);
        _colors = new u16[_colorCount];
        fread(_colors, 2, _colorCount, f);

        fread(&_frameCount, 1, 1, f);
        u16 tileCount = tileWidth * tileHeight;
        _tiles = new u8[64 * tileCount * _frameCount];
        fread(_tiles, 8 * 8 * tileCount * _frameCount, 1, f);

        fread(&_animationCount, 1, 1, f);
        _animations = new CSPRAnimation[_animationCount];
        for (int i = 0; i < _animationCount; i++) {
            int nameLen = str_len_file(f, 0);
            _animations[i].name = new char[nameLen + 1];
            fread(_animations[i].name, nameLen + 1, 1, f);
            fread(&_animations[i].frameCount, 1, 1, f);
            _animations[i].frames = new CSPRAnimFrame[_animations[i].frameCount];
            if (_animations[i].frameCount == 0) {
                // should free on error?
                return 4;
            }
            for (int j = 0; j < _animations[i].frameCount; j++) {
                fread(&_animations[i].frames[j].frame, 1, 1, f);
                fread(&_animations[i].frames[j].duration, 2, 1, f);
                fread(&_animations[i].frames[j].drawOffX, 1, 1, f);
                fread(&_animations[i].frames[j].drawOffY, 1, 1, f);
            }
        }

        _loaded = true;
        return 0;
    }

    void Texture::free_() {
        if (!_loaded)
            return;
        _loaded = false;
        delete[] _colors;
        _colors = nullptr;
        delete[] _tiles;
        _tiles = nullptr;
        if (_animations != nullptr) {
            for (int i = 0; i < _animationCount; i++) {
                delete[] _animations[i].name;
                _animations[i].name = nullptr;
                delete[] _animations[i].frames;
                _animations[i].frames = nullptr;
            }
            delete[] _animations;
        }
        _animations = nullptr;
    }
}