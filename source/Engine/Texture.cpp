#include "Engine/Texture.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"

namespace Engine {
    bool Texture::loadPath(const std::string& path) {
        std::string pathFull;
        std::string buffer;

        _path = path;

        pathFull = "nitro:/spr/" + path + ".cspr";

        FILE* f = fopen(pathFull.c_str(), "rb");
        if (!f) {
            buffer = "Error opening spr #r" + path;
            throw_(buffer);
        }

        loadCSPR(f);

        fclose(f);

        return true;
    }

    void Texture::loadCSPR(FILE *f) {
        free_();
        char header[4];
        u32 fileSize;
        u32 version;
        fread(header, 4, 1, f);

        const char expectedChar[4] = {'C', 'S', 'P', 'R'};
        if (memcmp(header, expectedChar, 4) != 0) {
            std::string buffer = "Error loading spr #r" + _path + "#x: Invalid header.";
            throw_(buffer);
        }

        fread(&fileSize, 4, 1, f);
        u32 pos = ftell(f);
        fseek(f, 0, SEEK_END);
        u32 size = ftell(f);
        fseek(f, pos, SEEK_SET);

        if (fileSize != size) {
            std::string buffer = "Error loading spr #r" + _path + "#x: File size doesn't match (expected: "
                    + std::to_string(fileSize) + ", actual: " + std::to_string(size) + ")";
            throw_(buffer);
        }

        fread(&version, 4, 1, f);
        if (version != CSPRHeader::version) {
            std::string buffer = "Error loading spr #r" + _path + "#x: Invalid version (expected: 6, actual: "
                    + std::to_string(version) + ")";
            throw_(buffer);
        }

        fread(&_width, 2, 1, f);
        fread(&_height, 2, 1, f);
        fread(&_topDownOffset, 2, 1, f);
        fread(&_frameCount, 1, 1, f);
        fread(&_hasOam, 1, 1, f);
        fread(&_has3D, 1, 1, f);

        u8 colorCount;
        fread(&colorCount, 1, 1, f);
        _colors.resize(colorCount);
        fread(&_colors[0], 2, colorCount, f);

        fread(&_animationCount, 1, 1, f);
        _animations.resize(_animationCount);
        for (int i = 0; i < _animationCount; i++) {
            int nameLen = str_len_file(f, 0);
            _animations[i].name.resize(nameLen);
            fread(&_animations[i].name[0], nameLen, 1, f);
            fseek(f, 1, SEEK_CUR);  // skip null character

            u8 frameCount;
            fread(&frameCount, 1, 1, f);

            _animations[i].frames.resize(frameCount);
            if (frameCount == 0) {
                // should free on error?
                std::string buffer = "Error loading spr #r" + _path + "#x: Animation " + std::to_string(i) +
                        " has no frames.";
                throw_(buffer);
            }
            for (int j = 0; j < frameCount; j++) {
                fread(&_animations[i].frames[j].frame, 1, 1, f);
                fread(&_animations[i].frames[j].duration, 2, 1, f);
                fread(&_animations[i].frames[j].drawOffX, 1, 1, f);
                fread(&_animations[i].frames[j].drawOffY, 1, 1, f);
            }
        }

        if (_hasOam) {
            if (colorCount > 15) {  // OAM can't have more than 15 colors
                std::string buffer = "Error loading spr #r" + _path + "#x: OAM can't be 8 bit.";
                throw_(buffer);
            }
            loadOam(f);
        }

        if (_has3D) {
            load3D(f);
        }

        _loaded = true;
    }

    void Texture::free_() {
        if (!_loaded)
            return;
        _loaded = false;
    }

    void Texture::loadOam(FILE *f) {
        fread(&_oamChunk.oamW, 1, 1, f);
        fread(&_oamChunk.oamH, 1, 1, f);
        _oamChunk.oamEntries.resize(_oamChunk.oamW * _oamChunk.oamH);
        for (int oamY = 0; oamY < _oamChunk.oamH; oamY++) {
            for (int oamX = 0; oamX < _oamChunk.oamW; oamX++) {
                auto &oamEntry = _oamChunk.oamEntries[oamY * _oamChunk.oamW + oamX];
                fread(&oamEntry.tilesW, 1, 1, f);
                fread(&oamEntry.tilesH, 1, 1, f);
                int tileDataSize = oamEntry.tilesW * oamEntry.tilesH * 32 * _frameCount;
                oamEntry.tilesFrameData.resize(tileDataSize);
                fread(&oamEntry.tilesFrameData[0], tileDataSize, 1, f);
            }
        }
    }

    void Texture::load3D(FILE *f) {
        fread(&_3dChunk.tilesAllocX, 1, 1, f);
        fread(&_3dChunk.tilesAllocY, 1, 1, f);
        _3dChunk.tiles.resize(_3dChunk.tilesAllocX * _3dChunk.tilesAllocY);
        for (int tileY = 0; tileY < _3dChunk.tilesAllocY; tileY++) {
            for (int tileX = 0; tileX < _3dChunk.tilesAllocX; tileX++) {
                auto & tile = _3dChunk.tiles[tileY * _3dChunk.tilesAllocX + tileX];
                fread(&tile.tileWidth, 1, 1, f);
                fread(&tile.tileHeight, 1, 1, f);
                int tileDataSize = tile.tileWidth * tile.tileHeight * 32 * _frameCount;
                if (_colors.size() > 15)
                    tileDataSize *= 2;
                tile.tileFrameData.resize(tileDataSize);
                fread(&tile.tileFrameData[0], tileDataSize, 1, f);
            }
        }
    }
}