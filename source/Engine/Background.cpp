//
// Created by cervi on 19/08/2022.
//
#include "Engine/Background.hpp"
#include "Engine/math.hpp"
#include "Engine/Engine.hpp"
#include "Engine/dma.hpp"

namespace Engine {
    s32 bg3ScrollX = 0, bg3ScrollY = 0;
    s16 bg3Pa = 0, bg3Pb = 0, bg3Pc = 0, bg3Pd = 0;

    bool Background::loadPath(std::string path) {
        std::string pathFull = "nitro:/bg/" + path + ".cbgf";
        _path = path;

        FILE* f = fopen(pathFull.c_str(), "rb");
        if (!f) {
            std::string buffer = "Error opening bg #r" + path;
            throw_(buffer);
        }

        loadCBGF(f);

        fclose(f);

        return true;
    }

    void Background::loadCBGF(FILE *f) {
        free_();
        char header[4];
        u32 fileSize;
        u32 version;
        u8 fileFormat;
        fread(header, 4, 1, f);
        u32 pos = ftell(f);
        fseek(f, 0, SEEK_END);
        u32 size = ftell(f);
        fseek(f, pos, SEEK_SET);

        const char expectedChar[4] = {'C', 'B', 'G', 'F'};
        if (memcmp(header, expectedChar, 4) != 0) {
            std::string buffer = "Error loading bg #r" + _path + "#x: Invalid header.";
            throw_(buffer);
        }

        fread(&fileSize, 4, 1, f);

        if (fileSize != size) {
            std::string buffer = "Error loading bg #r" + _path + "#x: File size doesn't match (expected: "
                                 + std::to_string(fileSize) + ", actual: " + std::to_string(size) + ")";
            throw_(buffer);
        }

        fread(&version, 4, 1, f);
        if (version != 1) {
            std::string buffer = "Error loading spr #r" + _path + "#x: Invalid version (expected: 1, actual: "
                                 + std::to_string(version) + ")";
            throw_(buffer);
        }

        fread(&fileFormat, 1, 1, f);
        _color8bit = fileFormat & 1;

        u8 colorCount;
        fread(&colorCount, 1, 1, f);
        if ((colorCount > 249 && _color8bit) || (colorCount > 15 && !_color8bit)) {
            std::string buffer = "Error loading bg #r" + _path + "#x: Color count does not match 8 bit flag (colors: "
                                 + std::to_string(colorCount) + ")";
            throw_(buffer);
        }

        _colors.resize(colorCount);
        fread(&_colors[0], 2, colorCount, f);

        fread(&_tileCount, 2, 1, f);

        u32 tileDataSize = 32;
        if (_color8bit)
            tileDataSize = 64;

        _tiles = std::unique_ptr<u8[]>(new u8[_tileCount * tileDataSize]);
        fread(_tiles.get(), tileDataSize, _tileCount, f);

        fread(&_width, 2, 1, f);
        fread(&_height, 2, 1, f);

        _map = std::unique_ptr<u16[]>(new u16[_width * _height]);
        fread(_map.get(), 2, _width * _height, f);

        _loaded = true;
    }

    void Background::free_() {
        if (!_loaded)
            return;
        _loaded = false;
    }

    int Background::loadBgTextMain() {
        videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        return loadBgTextEngine(&REG_BG3CNT, BG_PALETTE, BG_TILE_RAM(1),
                                BG_MAP_RAM(0));
    }

    int Background::loadBgTextSub() {
        videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
                        | (1 << 20));
        return loadBgTextEngine(&REG_BG3CNT_SUB, BG_PALETTE_SUB, BG_TILE_RAM_SUB(1),
                                BG_MAP_RAM_SUB(0));
    }

    int Background::loadBgTextEngine(vu16* bg3Reg, u16* paletteRam, u16* tileRam, u16* mapRam) {
        if (!_loaded)
            return 1;

        // Set control for 8-bit color depth
        *bg3Reg = (*bg3Reg & (~0x2080)) + (_color8bit << 7);

        // skip first color (2 bytes)
        dmaCopySafe(3, &_colors[0], (u8*)paletteRam + 2, 2 * _colors.size());

        u32 tileDataSize;
        if (_color8bit) {
            tileDataSize = 64;
        } else {
            tileDataSize = 32;
        }

        if (_tileCount > 1024)
            return 2;

        dmaCopySafe(3, _tiles.get(), tileRam, tileDataSize * _tileCount);

        u16 sizeFlag = 0;
        u16 mapRamUsage = 0x800;
        if (_width > 32) {
            sizeFlag += 1 << 14;  // bit 14 for 64 tile width
            mapRamUsage *= 2;
        }
        if (_height > 32) {
            sizeFlag += 1 << 15;  // bit 15 for 64 tile height
            mapRamUsage *= 2;
        }

        *bg3Reg = (*bg3Reg & (~0xC000)) + sizeFlag;
        dmaFillSafe(3, 0, mapRam, mapRamUsage);

        for (int mapX = 0; mapX < (_width + 31) / 32; mapX++) {
            int copyWidth = 32;
            if (mapX == (_width + 31) / 32)
                copyWidth = (_width + 31) - 32 * mapX;
            for (int mapY = 0; mapY < (_height + 31) / 32; mapY++) {
                u8* mapStart = (u8*)mapRam + (mapY * ((_width + 31) / 32) + mapX) * 2048;
                dmaFillSafe(3, 0, mapStart, 0x800);
                for (int row = mapY*32; row < _height && row < (mapY + 1) * 32; row++) {
                    dmaCopySafe(3, (u8 *)_map.get() + (row * _width + mapX * 32) * 2,
                                      mapStart + (row - mapY * 32) * 32 * 2, copyWidth * 2);
                }
            }
        }
        return 0;
    }

    int Background::loadBgExtendedMain(int forceSize) {
        videoSetMode(MODE_3_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        return loadBgExtendedEngine(&REG_BG3CNT, BG_PALETTE, BG_TILE_RAM(1),
                                    BG_MAP_RAM(0),
                                    &REG_BG3PA, &REG_BG3PB,
                                    &REG_BG3PC, &REG_BG3PD,
                                    forceSize);
    }

    int Background::loadBgExtendedSub(int forceSize) {
        videoSetModeSub(MODE_3_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
                        | (1 << 20));
        return loadBgExtendedEngine(&REG_BG3CNT_SUB, BG_PALETTE_SUB, BG_TILE_RAM_SUB(1),
                                    BG_MAP_RAM_SUB(0),
                                    &REG_BG3PA_SUB, &REG_BG3PB_SUB,
                                    &REG_BG3PC_SUB, &REG_BG3PD_SUB,
                                    forceSize);
    }

    int Background::loadBgExtendedEngine(vu16* bg3Reg, u16* paletteRam, u16* tileRam, u16* mapRam,
                                         vs16* reg3A, vs16* reg3B, vs16* reg3C, vs16* reg3D,
                                         int forceSize) {
        if (!_loaded)
            return 1;

        // Clear control for 16-bit bg map
        // Can't set 4 bit on extended
        *bg3Reg = (*bg3Reg & (~0x2080)) | (1 << 13);

        // skip first color (2 bytes)
        dmaCopySafe(3, &_colors[0], (u8 *) paletteRam + 2, 2 * _colors.size());

        u16 sizeFlag = 0;
        u16 mapRamUsage = 0x200;
        u8 mapW = _width, mapH = _height;
        if (forceSize != 0) {
            mapW = forceSize;
            mapH = forceSize;
        }
        if (mapW > 64 || mapH > 64) {
            sizeFlag = 3;
        } else if (mapW > 32 || mapH >= 32) {
            sizeFlag = 2;
        } else if (mapW > 16 || mapH > 16) {
            sizeFlag = 1;
        }
        mapRamUsage <<= 2 * sizeFlag;

        *bg3Reg = (*bg3Reg & (~0xC000)) + (sizeFlag << 14);
        *reg3A = (1 << 8);
        *reg3B = 0;
        *reg3C = 0;
        *reg3D = (1 << 8);
        dmaFillSafe(3, 0, mapRam, mapRamUsage);

        // loadBgRectEngine(bg3Reg, tileRam, mapRam, -1, -1, 34, 26);
        // An extended engine will need a load bg rect afterwards
        return 0;
    }

    void clearMain() {
        videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        clearEngine(&REG_BG3CNT, BG_TILE_RAM(1), BG_MAP_RAM(0));
    }

    void clearSub() {
        videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
                        | (1 << 20));
        clearEngine(&REG_BG3CNT_SUB, BG_TILE_RAM_SUB(1), BG_MAP_RAM_SUB(0));
    }

    void clearEngine(vu16* bg3Reg, u16* tileRam, u16* mapRam) {
        u16 mapRamUsage = 0x800;
        dmaFillSafe(3, 0, mapRam, mapRamUsage);
        *bg3Reg = (*bg3Reg & (~0xE080)); // size 32x32
        *tileRam = 0;
    }

    int Background::loadBgRectMain(int x, int y, int w, int h) {
        return loadBgRectEngine(&REG_BG3CNT, BG_TILE_RAM(1), BG_MAP_RAM(0), x, y, w, h);
    }

    int Background::loadBgRectSub(int x, int y, int w, int h) {
        return loadBgRectEngine(&REG_BG3CNT_SUB, BG_TILE_RAM_SUB(1),
                                BG_MAP_RAM_SUB(0), x, y, w, h);
    }

    int Background::loadBgRectEngine(const vu16* bg3Reg, u16* tileRam, u16* mapRam,
                                     int x, int y, int w, int h) {
        if (!_loaded)
            return 1;

        int mapSize = 16 << ((*bg3Reg >> 14) & 3);
        for (int row = y; row < y + h; row++) {
            for (int col = x; col < x + w; col++) {
                int dstRow = mod(row, mapSize);
                int dstCol = mod(col, mapSize);
                auto* mapRes = (u16*)((u8*)mapRam + (dstRow * mapSize + dstCol) * 2);
                int tileDst = mod(row, 26) * 34 + mod(col, 34);
                *mapRes = tileDst;
                auto* tileRes = (u16*)((u8*)tileRam + tileDst * 64);
                if (0 > row or row >= _height or 0 > col or col >= _width) {
                    dmaFillSafe(3, 0, tileRes, 64);
                    continue;
                }
                int srcRow = row;
                int srcCol = col;
                auto* mapSrc = (u16*)((u8 *)_map.get() + (srcRow * _width + srcCol) * 2);

                if (_color8bit) {
                    u8 *src = (u8 *)_tiles.get() + (*mapSrc) * 64;
                    dmaCopySafe(3, src, tileRes, 64);
                }
                else {
                    u8 *src = (u8 *)_tiles.get() + (*mapSrc) * 32;
                    for (int i = 0; i < 64; i++) {
                        bool highBits = i & 1;
                        tileRes[i / 2] &= ~(0xFF << (8 * highBits));
                        tileRes[i / 2] |= (src[i / 2] >> (4 * highBits) & 0xF) << (8 * highBits);
                    }
                }
            }
        }
        return 0;
    }
}