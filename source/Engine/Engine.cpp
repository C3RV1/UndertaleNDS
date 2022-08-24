#include "Engine/Engine.hpp"

namespace Engine {
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

        lcdMainOnTop();

        vramSetBankA(VRAM_A_TEXTURE_SLOT0);
        vramSetBankB(VRAM_B_TEXTURE_SLOT1);
        vramSetBankC(VRAM_C_SUB_BG);
        vramSetBankD(VRAM_D_SUB_SPRITE);
        vramSetBankE(VRAM_E_MAIN_BG);
        vramSetBankF(VRAM_F_MAIN_BG_0x06010000);
        vramSetBankG(VRAM_G_TEX_PALETTE_SLOT0);
        // Bank H unused
        vramSetBankI(VRAM_I_LCD);  // Enabled but unused for now

        videoSetMode(MODE_0_3D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_0_2D | DISPLAY_BG1_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_1D | DISPLAY_SPR_ACTIVE
        | (1 << 20));

        REG_BG0CNT = BG_PRIORITY(1);
        GFX_CLEAR_COLOR = 0;

        REG_BG1CNT = BG_PRIORITY(2) | BG_TILE_BASE(4) | BG_MAP_BASE(1);
        memset(BG_TILE_RAM(4), 0, 1);
        memset(BG_MAP_RAM(1), 0, 32 * 32 * 2);
        REG_BG3CNT = BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0);

        REG_BG1CNT_SUB = BG_PRIORITY(2) | BG_TILE_BASE(4) | BG_MAP_BASE(1);
        memset(BG_TILE_RAM_SUB(4), 0, 1);
        memset(BG_MAP_RAM_SUB(1), 0, 32 * 32 * 2);
        REG_BG3CNT_SUB = BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0);

        // Init 3d
        MATRIX_CONTROL = 0; // Select projection matrix
        MATRIX_IDENTITY = 0;  // Load identity matrix
        MATRIX_STORE = 0;  // Push matrix to identity
        while (GFX_BUSY); // wait for cmd execution
        return 0;
    }

    void tick() {
        if (BGM::shouldClose) {
            BGM::stopWAV();
        }
        glFlush(0);
        swiWaitForVBlank();
        scanKeys();
    }

    int loadBgMain(Background& bg) {
        return loadBgEngine(bg, &REG_BG3CNT, BG_PALETTE, BG_TILE_RAM(1),
                            BG_MAP_RAM(0));
    }

    int loadBgSub(Background& bg) {
        return loadBgEngine(bg, &REG_BG3CNT_SUB, BG_PALETTE_SUB, BG_TILE_RAM_SUB(1),
                            BG_MAP_RAM_SUB(0));
    }

    int loadBgEngine(Background& bg, vu16* bg3Reg, u16* paletteRam,
                       u16* tileRam, u16* mapRam) {
        if (!bg.getLoaded())
            return 1;
        bool color8bit = bg.getColor8bit();

        // Set control for 8-bit color depth
        *bg3Reg = (*bg3Reg & (~0x80)) + (color8bit << 7);

        // skip first color (2 bytes)
        dmaCopy(bg.getColors(), (uint8_t*)paletteRam + 2, 2 * bg.getColorCount());

        uint32_t tileDataSize;
        if (color8bit) {
            tileDataSize = 64;
        } else {
            tileDataSize = 32;
        }

        dmaCopyWords(3, bg.getTiles(), tileRam, tileDataSize * bg.getTileCount());

        uint16_t sizeFlag = 0;
        uint16_t mapRamUsage = 0x800;
        uint8_t width, height;
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

        for (int row = 0; row < height; row++) {
            dmaCopyHalfWords(3, (uint8_t*) bg.getMap() + row * width * 2,
                             (uint8_t*) mapRam + row * 32 * 2, width * 2);
        }
        return 0;
    }
}
