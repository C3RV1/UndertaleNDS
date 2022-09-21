#include "Engine/Engine.hpp"

namespace Engine {
    int32_t bg3ScrollX = 0, bg3ScrollY = 0;
    int16_t bg3Pa = 0, bg3Pb = 0, bg3Pc = 0, bg3Pd = 0;

    int init() {
        powerOn(POWER_ALL);
        if (!nitroFSInit(nullptr)) {
            nocashMessage("nitroFSInit failure!\n");
            return -1;
        }

        mm_ds_system sys;
        sys.mod_count 			= 0;
        sys.samp_count			= 0;
        sys.mem_bank			= nullptr;
        sys.fifo_channel		= FIFO_MAXMOD;
        mmInit( &sys );

        BGM::initAudioStream();

        lcdMainOnTop();

        vramSetBankA(VRAM_A_MAIN_BG_0x06000000);
        vramSetBankB(VRAM_B_TEXTURE_SLOT0);
        vramSetBankC(VRAM_C_SUB_BG);
        vramSetBankD(VRAM_D_SUB_SPRITE);
        vramSetBankE(VRAM_E_TEX_PALETTE);
        vramSetBankF(VRAM_F_LCD);
        vramSetBankG(VRAM_G_LCD);
        // Bank H unused
        vramSetBankI(VRAM_I_LCD);  // Enabled but unused for now

        videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
        | (1 << 20));

        REG_BG0CNT = BG_PRIORITY(1);
        GFX_CLEAR_COLOR = 0;

        REG_BG1CNT = BG_PRIORITY(1) | BG_TILE_BASE(5) | BG_MAP_BASE(4);
        memset(BG_TILE_RAM(5), 0, 1);
        memset(BG_MAP_RAM(4), 0, 32 * 32 * 2);
        REG_BG3CNT = BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0);

        REG_BG1CNT_SUB = BG_PRIORITY(1) | BG_TILE_BASE(5) | BG_MAP_BASE(1);
        memset(BG_TILE_RAM_SUB(5), 0, 1);
        memset(BG_MAP_RAM_SUB(1), 0, 32 * 32 * 2);
        REG_BG3CNT_SUB = BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0);

        // Init 3d
        GFX_CONTROL |= 1 | (1 << 3);
        glClearDepth( GL_MAX_DEPTH );
        glViewport(0, 0, 255, 191);
        glMatrixMode( GL_PROJECTION );     // set matrixmode to projection
        glLoadIdentity();				 // reset
        glOrthof32( 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, -1 << 12, 1 << 12 );  // downscale projection matrix
        return 0;
    }

    void tick() {
        main3dSpr.draw();
        glFlush(0);
        swiWaitForVBlank();
        REG_BG3X = bg3ScrollX;
        REG_BG3Y = bg3ScrollY;
        REG_BG3PA = bg3Pa;
        REG_BG3PB = bg3Pb;
        REG_BG3PC = bg3Pc;
        REG_BG3PD = bg3Pd;
        // Render post v-blank
        // REG_DISPCNT_SUB |= (1 << 7) | (1 << 5);
        REG_DISPCNT_SUB |= (1 << 7);
        OAMManagerSub.draw();  // Update oam in v-blank
        REG_DISPCNT_SUB &= ~(1 << 7);
        main3dSpr.updateTextures();  // Update textures in v-blank
        mmStreamUpdate();
        scanKeys();
    }

    int loadBgTextMain(Background& bg) {
        videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        return loadBgTextEngine(bg, &REG_BG3CNT, BG_PALETTE, BG_TILE_RAM(1),
                                BG_MAP_RAM(0));
    }

    int loadBgTextSub(Background& bg) {
        videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
                        | (1 << 20));
        return loadBgTextEngine(bg, &REG_BG3CNT_SUB, BG_PALETTE_SUB, BG_TILE_RAM_SUB(1),
                                BG_MAP_RAM_SUB(0));
    }

    int loadBgTextEngine(Background& bg, vu16* bg3Reg, u16* paletteRam,
                         u16* tileRam, u16* mapRam) {
        if (!bg.getLoaded())
            return 1;
        bool color8bit = bg.getColor8bit();

        // Set control for 8-bit color depth
        *bg3Reg = (*bg3Reg & (~0x2080)) + (color8bit << 7);

        // skip first color (2 bytes)
        dmaCopy(bg.getColors(), (uint8_t*)paletteRam + 2, 2 * bg.getColorCount());

        uint32_t tileDataSize;
        if (color8bit) {
            tileDataSize = 64;
        } else {
            tileDataSize = 32;
        }

        if (bg.getTileCount() > 1024)
            return 2;

        dmaCopyWords(3, bg.getTiles(), tileRam, tileDataSize * bg.getTileCount());

        uint16_t sizeFlag = 0;
        uint16_t mapRamUsage = 0x800;
        uint16_t width, height;
        bg.getSize(width, height);
        if (width > 32) {
            sizeFlag += 1 << 14;  // bit 14 for 64 tile width
            mapRamUsage *= 2;
        }
        if (height > 32) {
            sizeFlag += 1 << 15;  // bit 15 for 64 tile height
            mapRamUsage *= 2;
        }

        *bg3Reg = (*bg3Reg & (~0xC000)) + sizeFlag;
        memset(mapRam, 0, mapRamUsage);

        for (int mapX = 0; mapX < (width + 31) / 32; mapX++) {
            int copyWidth = 32;
            if (mapX == (width + 31) / 32)
                copyWidth = (width + 31) - 32 * mapX;
            for (int mapY = 0; mapY < (height + 31) / 32; mapY++) {
                uint8_t* mapStart = (uint8_t*)mapRam + (mapY * ((width + 31) / 32) + mapX) * 2048;
                memset(mapStart, 0, 0x800);
                for (int row = mapY*32; row < height && row < (mapY + 1) * 32; row++) {
                    dmaCopyHalfWords(3, (uint8_t *) bg.getMap() + (row * width + mapX * 32) * 2,
                                     mapStart + (row - mapY * 32) * 32 * 2, copyWidth * 2);
                }
            }
        }
        return 0;
    }

    int loadBgExtendedMain(Background& bg, int forceSize) {
        videoSetMode(MODE_3_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        return loadBgExtendedEngine(bg, &REG_BG3CNT, BG_PALETTE, BG_TILE_RAM(1),
                                    BG_MAP_RAM(0),
                                    &REG_BG3PA, &REG_BG3PB,
                                    &REG_BG3PC, &REG_BG3PD,
                                    forceSize);
    }

    int loadBgExtendedSub(Background& bg, int forceSize) {
        videoSetModeSub(MODE_3_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
                        | (1 << 20));
        return loadBgExtendedEngine(bg, &REG_BG3CNT_SUB, BG_PALETTE_SUB, BG_TILE_RAM_SUB(1),
                                    BG_MAP_RAM_SUB(0),
                                    &REG_BG3PA_SUB, &REG_BG3PB_SUB,
                                    &REG_BG3PC_SUB, &REG_BG3PD_SUB,
                                    forceSize);
    }

    int loadBgExtendedEngine(Background& bg, vu16* bg3Reg, u16* paletteRam,
                             u16* tileRam, u16* mapRam,
                             vs16* reg3A, vs16* reg3B,
                             vs16* reg3C, vs16* reg3D,
                             int forceSize) {
        if (!bg.getLoaded())
            return 1;

        // Clear control for 16-bit bg map
        // Can't set 4 bit on extended
        *bg3Reg = (*bg3Reg & (~0x2080)) | (1 << 13);

        // skip first color (2 bytes)
        dmaCopy(bg.getColors(), (uint8_t *) paletteRam + 2, 2 * bg.getColorCount());

        uint16_t sizeFlag = 0;
        uint16_t mapRamUsage = 0x200;
        uint16_t width, height;
        bg.getSize(width, height);
        uint8_t mapW = width, mapH = height;
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

        loadBgRectEngine(bg, bg3Reg, tileRam, mapRam, -1, -1, 34, 26);
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
        uint16_t mapRamUsage = 0x800;
        memset(mapRam, 0, mapRamUsage);
        *bg3Reg = (*bg3Reg & (~0xE080)); // size 32x32
        *tileRam = 0;
    }

    int loadBgRectMain(Background& bg, int x, int y, int w, int h) {
        return loadBgRectEngine(bg, &REG_BG3CNT, BG_TILE_RAM(1), BG_MAP_RAM(0), x, y, w, h);
    }

    int loadBgRectSub(Background& bg, int x, int y, int w, int h) {
        return loadBgRectEngine(bg, &REG_BG3CNT_SUB, BG_TILE_RAM_SUB(1),
                                BG_MAP_RAM_SUB(0), x, y, w, h);
    }

    int loadBgRectEngine(Background& bg, const vu16* bg3Reg, u16* tileRam, u16* mapRam, int x, int y,
                         int w, int h) {
        if (!bg.getLoaded())
            return 1;

        bool color8bit = bg.getColor8bit();

        int mapSize = 16 << ((*bg3Reg >> 14) & 3);
        uint16_t width, height;
        bg.getSize(width, height);
        for (int row = y; row < y + h; row++) {
            for (int col = x; col < x + w; col++) {
                int srcRow = mod(row, height);
                int srcCol = mod(col, width);
                int dstRow = mod(row, mapSize);
                int dstCol = mod(col, mapSize);
                auto* mapRes = (uint16_t*)((uint8_t*)mapRam + (dstRow * mapSize + dstCol) * 2);
                int tileDst = mod(row, 26) * 34 + mod(col, 34);
                *mapRes = tileDst;
                auto* tileRes = (uint16_t*)((uint8_t*)tileRam + tileDst * 64);
                auto* mapSrc = (uint16_t*)((uint8_t *) bg.getMap() + (srcRow * width + srcCol) * 2);

                if (color8bit) {
                    uint8_t *src = (uint8_t *) bg.getTiles() + (*mapSrc) * 64;
                    dmaCopyHalfWords(3, src, tileRes, 64);
                }
                else {
                    uint8_t *src = (uint8_t *) bg.getTiles() + (*mapSrc) * 32;
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
