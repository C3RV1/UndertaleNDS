//
// Created by cervi on 19/08/2022.
//
#include "Engine/Background.hpp"
#include "Engine/math.hpp"

namespace Engine {
    s32 bg3ScrollX = 0, bg3ScrollY = 0;
    s16 bg3Pa = 0, bg3Pb = 0, bg3Pc = 0, bg3Pd = 0;

    bool Background::loadPath(const char *path) {
        char pathFull[100];
        char buffer[100];

        sprintf(pathFull, "nitro:/bg/%s.cbgf", path);

        FILE* f = fopen(pathFull, "rb");
        if (!f) {
            sprintf(buffer, "Error opening bg %s", path);
            nocashMessage(buffer);
            return false;
        }

        int loadRes = loadCBGF(f);

        fclose(f);

        if (loadRes != 0) {
            sprintf(buffer, "Error loading bg %s: %d", path, loadRes);
            nocashMessage(buffer);
            return false;
        }

        return true;
    }

    int Background::loadCBGF(FILE *f) {
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
        _color8bit = fileFormat & 1;

        fread(&_colorCount, 1, 1, f);
        if ((_colorCount > 249 && _color8bit) || (_colorCount > 15 && !_color8bit)) {
            return 4;
        }

        _colors = new u16[_colorCount];
        fread(_colors, 2, _colorCount, f);

        fread(&_tileCount, 2, 1, f);

        u32 tileDataSize = 32;
        if (_color8bit)
            tileDataSize = 64;

        _tiles = new u8[_tileCount * tileDataSize];
        fread(_tiles, tileDataSize, _tileCount, f);

        fread(&_width, 2, 1, f);
        fread(&_height, 2, 1, f);

        _map = new u16[_width * _height];
        fread(_map, 2, _width * _height, f);

        _loaded = true;
        return 0;
    }

    void Background::free_() {
        if (!_loaded)
            return;
        _loaded = false;
        delete[] _colors;
        _colors = nullptr;
        delete[] _tiles;
        _tiles = nullptr;
        delete[] _map;
        _map = nullptr;
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
        dmaCopy(_colors, (u8*)paletteRam + 2, 2 * _colorCount);

        u32 tileDataSize;
        if (_color8bit) {
            tileDataSize = 64;
        } else {
            tileDataSize = 32;
        }

        if (_tileCount > 1024)
            return 2;

        memcpy(tileRam, _tiles, tileDataSize * _tileCount);

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
        memset(mapRam, 0, mapRamUsage);

        for (int mapX = 0; mapX < (_width + 31) / 32; mapX++) {
            int copyWidth = 32;
            if (mapX == (_width + 31) / 32)
                copyWidth = (_width + 31) - 32 * mapX;
            for (int mapY = 0; mapY < (_height + 31) / 32; mapY++) {
                u8* mapStart = (u8*)mapRam + (mapY * ((_width + 31) / 32) + mapX) * 2048;
                memset(mapStart, 0, 0x800);
                for (int row = mapY*32; row < _height && row < (mapY + 1) * 32; row++) {
                    dmaCopyHalfWords(3, (u8 *) _map + (row * _width + mapX * 32) * 2,
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
        dmaCopy(_colors, (u8 *) paletteRam + 2, 2 * _colorCount);

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
        memset(mapRam, 0, mapRamUsage);

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
        memset(mapRam, 0, mapRamUsage);
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
                    memset(tileRes, 0, 64);
                    continue;
                }
                int srcRow = row;
                int srcCol = col;
                auto* mapSrc = (u16*)((u8 *) _map + (srcRow * _width + srcCol) * 2);

                if (_color8bit) {
                    u8 *src = (u8 *) _tiles + (*mapSrc) * 64;
                    dmaCopyHalfWords(3, src, tileRes, 64);
                }
                else {
                    u8 *src = (u8 *) _tiles + (*mapSrc) * 32;
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