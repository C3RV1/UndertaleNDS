//
// Created by cervi on 19/08/2022.
//

#ifndef UNDERTALE_BACKGROUND_HPP
#define UNDERTALE_BACKGROUND_HPP

#include <cstdio>
#include <cstring>

#define ARM9
#include <nds.h>

namespace Engine {
    class Background {
    public:
        bool loadPath(const char* path);
        int loadCBGF(FILE* f);
        bool getLoaded() const { return _loaded; }
        void getSize(u16& width, u16& height) const {
            width = _width;
            height = _height;
        }
        void free_();
        ~Background() { free_(); }

        int loadBgTextMain();
        int loadBgTextSub();

        int loadBgExtendedMain(int forceSize);
        int loadBgExtendedSub(int forceSize);

        int loadBgRectMain(int x, int y, int w, int h);
        int loadBgRectSub(int x, int y, int w, int h);
    private:
        bool _loaded = false;
        bool _color8bit = false;
        u8 _colorCount = 0;
        u16* _colors = nullptr;
        u16 _tileCount = 0;
        u8* _tiles = nullptr;
        u16 _width = 0, _height = 0;
        u16* _map = nullptr;

        int loadBgTextEngine(vu16* bg3Reg, u16* paletteRam, u16* tileRam, u16* mapRam);
        int loadBgExtendedEngine(vu16* bg3Reg, u16* paletteRam, u16* tileRam, u16* mapRam,
                                 vs16* reg3A, vs16* reg3B, vs16* reg3C, vs16* reg3D,
                                 int forceSize);
        int loadBgRectEngine(const vu16* bg3Reg, u16* tileRam, u16* mapRam,
                             int x, int y, int w, int h);
    };

    void clearMain();
    void clearSub();
    void clearEngine(vu16* bg3Reg, u16* tileRam, u16* mapRam);

    extern s32 bg3ScrollX, bg3ScrollY;
    extern s16 bg3Pa, bg3Pb, bg3Pc, bg3Pd;
}

#endif //UNDERTALE_BACKGROUND_HPP
