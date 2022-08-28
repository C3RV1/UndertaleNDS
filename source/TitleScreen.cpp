//
// Created by cervi on 22/08/2022.
//
#include "TitleScreen.hpp"

void runTitleScreen() {
    const int fadeInFrames = 30;
    const int holdFrames = 60*5;
    int fadeOutFrames = 30;  // replace on last frame to fadeOutLastFrames
    const int height = 350;
    const int holdLastFrames = 60 * 4;
    const int fadeOutLastFrames = 60 * 4;
    const int scrollFrames = 60 * 7;
    const int introLogoFrames = 300;
    const int textX = 30;
    const int textX_alt = 25;
    const int textX_centered = 256 / 2 - 43;
    const int textY = 30;
    const int lineSpacing = 20;
    const int characterExtraSpacing = 3;
    const int characterExtraSpacing_intro3 = 2;
    const int dotFrames = 40;
    const int otherPunctuationFrames = 40;
    const int letterFrames = 4;
    const int pressAButtonX = 40;
    const int pressAButtonY = 60;
    int timer;

    char textBuffer[100];
    FILE* textStream = fopen("nitro:/data/intro.txt", "rb");
    if (textStream == nullptr)
        nocashMessage("Error opening intro text");

    Engine::Background currentBackground;
    char buffer[100];
    FILE *f;

    Engine::Font mainFont;
    f = fopen("nitro:/fnt/fnt_main.font.cfnt", "rb");
    if (f) {
        int font_load = mainFont.loadCFNT(f);
        if (font_load != 0) {
            sprintf(buffer, "Error loading font: %d", font_load);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening font file");
    }
    fclose(f);

    int loadWavResult = BGM::globalWAV.loadWAV("nitro:/z_audio/mus_story_mod.wav");
    sprintf(buffer, "LOAD WAV story %d", loadWavResult);
    nocashMessage(buffer);
    BGM::playWAV(BGM::globalWAV);

    setBrightness(1, -16);
    bool skip = false;

    for (int introIdx = 0; introIdx < 11 && !skip; introIdx++) {
        sprintf(buffer, "nitro:/bg/intro/intro%d.cbgf", introIdx);
        f = fopen(buffer, "rb");
        if (f) {
            int bgLoad = currentBackground.loadCBGF(f);
            if (bgLoad != 0) {
                sprintf(buffer, "Error loading bg/intro/intro%d.cbgf: %d",
                        introIdx, bgLoad);
                nocashMessage(buffer);
            }
        } else {
            sprintf(buffer, "Error opening bg/intro/intro%d.cbgf", introIdx);
            nocashMessage(buffer);
        }
        fclose(f);

        Engine::loadBgTextMain(currentBackground);
        if (introIdx == 10)  // Intro last has scrolling
            REG_BG3VOFS = height-192;

        timer = fadeInFrames;
        while (timer >= 0 && introIdx != 0 && !skip) {
            Engine::tick();
            skip = keysDown() != 0;
            setBrightness(1, (-16 * timer) / fadeInFrames);
            timer--;
        }
        setBrightness(1, 0);

        if (introIdx == 10) {  // Intro last has longer hold
            timer = holdLastFrames;
        } else {
            timer = holdFrames;
        }

        int textTimer = letterFrames;

        if (f) {
            int textLen = strlen_file(textStream, '@');
            fread(textBuffer, textLen + 2, 1, textStream);  // read @\n characters
            textBuffer[textLen] = '\0'; // replace @ terminator with 0 byte
        } else {
            *textBuffer = 0;  // if file couldn't be opened don't write anything
        }

        char* textPointer = textBuffer;
        int initialX = textX;
        if (introIdx == 3 || introIdx == 6)  // Fit to screen
            initialX = textX_alt;
        else if (introIdx == 5)  // MT EBOTT. centered
            initialX = textX_centered;
        int x = initialX, y = textY;
        Engine::textSub.clear();
        while (timer >= 0 && !skip) {
            Engine::tick();
            skip = keysDown() != 0;
            if (textTimer == 0 && *textPointer != 0) {
                char glyph = *textPointer++;
                if (glyph == '\n') {
                    x = initialX;
                    y += lineSpacing;
                } else {
                    Engine::textSub.drawGlyph(mainFont, glyph, x, y);
                    if (introIdx == 3)  // Fit to screen
                        x += characterExtraSpacing_intro3;
                    else
                        x += characterExtraSpacing;
                }
                textTimer = letterFrames;
                if (glyph == '.' && introIdx != 5)  // Don't want time on MT. EBOTT
                    textTimer = dotFrames;
                else if (glyph == ',' || glyph == ':' || glyph == ';')
                    textTimer = otherPunctuationFrames;
            }
            timer--;
            if (*textPointer != 0)
                textTimer--;
        }

        if (introIdx == 10) {  // Intro last scroll
            timer = scrollFrames;
            while (timer >= 0 && !skip) {
                Engine::tick();
                skip = keysDown() != 0;
                REG_BG3VOFS = ((height-192) * timer) / scrollFrames;
                timer--;
            }

            timer = holdLastFrames;
            while (timer >= 0 && !skip) {
                Engine::tick();
                skip = keysDown() != 0;
                timer--;
            }
        }

        if (introIdx == 10)  // Intro last has longer fade out
            fadeOutFrames = fadeOutLastFrames;  // Can change value as it's never used again
        timer = fadeOutFrames;
        while (timer >= 0 && !skip) {
            Engine::tick();
            skip = keysDown() != 0;
            setBrightness(1, (-16 * (fadeOutFrames - timer)) / fadeOutFrames);
            if (introIdx == 6)
                setBrightness(2, (-16 * (fadeOutFrames - timer)) / fadeOutFrames);
            timer--;
        }
    }

    REG_BG3VOFS = 0;
    Engine::textSub.clear();

    if (skip) {
        fclose(textStream);
        BGM::stopWAV();
        BGM::globalWAV.free_();
        return;
    }
    loadWavResult = BGM::globalWAV.loadWAV("nitro:/z_audio/mus_intronoise.wav");
    sprintf(buffer, "LOAD WAV introsound %d", loadWavResult);
    nocashMessage(buffer);
    BGM::playWAV(BGM::globalWAV);

    f = fopen("nitro:/bg/intro/title.cbgf", "rb");
    if (f) {
        int titleLoad = currentBackground.loadCBGF(f);
        if (titleLoad != 0) {
            sprintf(buffer, "Error loading bg/intro/title.cbgf: %d", titleLoad);
            nocashMessage(buffer);
        }
    } else {
        nocashMessage("Error opening bg/intro/title.cbgf");
    }
    fclose(f);

    Engine::loadBgTextMain(currentBackground);
    setBrightness(3, 0);  // set brightness to full bright

    timer = introLogoFrames;
    int textLen = strlen_file(textStream, '@');
    fread(textBuffer, textLen + 2, 1, textStream);
    textBuffer[textLen] = '\0';
    while (!skip) {
        Engine::tick();
        skip = keysDown() != 0;
        if (timer > 0) {
            timer--;
            if (timer <= 0) {
                const char* textPointer = textBuffer;
                int x = pressAButtonX, y = pressAButtonY;
                while (*textPointer != 0) {
                    Engine::textSub.drawGlyph(mainFont, *textPointer, x, y);
                    textPointer++;
                }
            }
        }
    }
    fclose(textStream);
    BGM::stopWAV();
    BGM::globalWAV.free_();
}
