#include "Engine/Engine.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite3DManager.hpp"
#include "Engine/OAMManager.hpp"
#include "filesystem.h"

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

        Audio::initAudioStream();

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
        glMatrixMode( GL_PROJECTION );     // set matrix mode to projection
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
        REG_DISPCNT_SUB |= (1 << 7);
        OAMManagerSub.draw();  // Update oam in v-blank
        REG_DISPCNT_SUB &= ~(1 << 7);
        main3dSpr.updateTextures();  // Update textures in v-blank
        mmStreamUpdate();
        scanKeys();
    }
}
