#include "Engine/Engine.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite3DManager.hpp"
#include "Engine/OAMManager.hpp"
#include "Engine/dma.hpp"

#ifndef BLOCKSDS_SDK
#include "nitrofs.h"
#else
#include <filesystem.h>
#endif

namespace Engine {
    int init() {
        powerOn(POWER_ALL);
        if (!nitroFSInit(nullptr)) {
            nocashMessage("nitroFSInit failure!\n");
            return -1;
        }

        srand(time(nullptr));

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
        dmaFillSafe(3, 0, BG_TILE_RAM(5), 64);
        dmaFillSafe(3, 0, BG_MAP_RAM(4), 32 * 32 * 2);
        REG_BG3CNT = BG_PRIORITY(3) | BG_TILE_BASE(1) | BG_MAP_BASE(0);

        REG_BG1CNT_SUB = BG_PRIORITY(1) | BG_TILE_BASE(5) | BG_MAP_BASE(1);
        dmaFillSafe(3, 0, BG_TILE_RAM_SUB(5), 64);
        dmaFillSafe(3, 0, BG_MAP_RAM_SUB(1), 32 * 32 * 2);
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
        Audio2::audioManager.update();
        swiWaitForVBlank();
        // TODO: Scroll and bg3 negative? Sub screen?
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
        scanKeys();
    }

    [[noreturn]] void throw_(std::string message) {
        static bool handlingException = false;
        nocashMessage("Exception caught:");
        nocashMessage(message.c_str());
        if (handlingException) {
            nocashMessage("Recursive call to throw_");
            while (true);
        }
        handlingException = true;
        lcdMainOnBottom();
        setBrightness(1, 0);
        textMain.clear();
        clearMain();
        Font system_font;
        system_font.loadPath("fnt_maintext.font");

        message = "#rCAUGHT EXCEPTION:#x\n" + message;

        constexpr int spacing = 10;
        constexpr int lineSpacing = 20;
        int x = spacing, y = spacing;
        bool command = false;

        textMain.setPaletteColor(1, 255, 255, 255, true);
        textMain.setPaletteColor(2, 255, 30, 30, true);

        for (auto const & message_char : message) {
            if (message_char == '\n') {
                x = spacing;
                y += lineSpacing;
                continue;
            }
            else if (message_char == '#') {
                command = true;
                continue;
            }
            if (command) {
                if (message_char == 'r') {
                    textMain.setColor(2);
                } else if (message_char == 'x') {
                    textMain.setColor(1);
                }
                command = false;
                continue;
            }
            if (system_font.getGlyphWidth(message_char) + x >= 256 - spacing) {
                x = spacing * 4;
                y += lineSpacing;
            }
            textMain.drawGlyph(system_font, message_char, x, y);
        }

        while (true) {
            Engine::tick();
        }

    }
}
