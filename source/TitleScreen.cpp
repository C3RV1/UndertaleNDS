//
// Created by cervi on 22/08/2022.
//
#include "TitleScreen.hpp"

void runTitleScreen() {
    const int fadeInFrames = 30;
    const int holdFrames = 60*5;
    int fadeOutFrames = 30;
    const int height = 350;
    const int holdLastFrames = 60 * 4;
    const int scrollFrames = 60 * 7;
    const int introLogoFrames = 300;
    const char* pressText = "[Press any button]";
    int timer;

    const char* texts[] = {
            "Long ago, two races\nruled over Earth:\nHUMANS and MONSTERS.",
            "One day, war broke\nout between the two\nraces.",
            "After a long battle,\nthe humans were\nvictorious.",
            "They sealed the monsters\nunderground with a magic\nspell.",
            "Many years later...",
            "MT. EBOTT\n    201X",
            "Legends say that those\nwho climb the mountain\nnever return.",
            "",
            "",
            "",
            ""
    };
    int letterFrames = 4;

    Engine::Background currentBackground;
    char buffer[100];
    FILE *f;

    Engine::Font mainFont;
    f = fopen("fnt/fnt_main.font.cfnt", "rb");
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

    BGM::WAV music, introSound;
    int loadWavResult = music.loadWAV(fopen("audio/mus_story_mod.wav", "rb"));
    int loadSoundResult = introSound.loadWAV(fopen("audio/mus_intronoise.wav", "rb"));
    sprintf(buffer, "LOAD WAVs %d %d", loadWavResult, loadSoundResult);
    nocashMessage(buffer);
    BGM::playWAV(music);

    setBrightness(1, -16);
    Engine::tick();
    bool skip = false;

    for (int introIdx = 0; introIdx < 11 && !skip; introIdx++) {
        sprintf(buffer, "bg/intro/intro%d.cbgf", introIdx);
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

        Engine::loadBgMain(currentBackground);
        if (introIdx == 10)  // Intro last has scrolling
            REG_BG3VOFS = height-192;

        timer = fadeInFrames;
        while (timer >= 0 && introIdx != 0 && !skip) {
            skip = keysDown() != 0;
            setBrightness(1, (-16 * timer) / fadeInFrames);
            Engine::tick();
            timer--;
        }
        setBrightness(1, 0);

        if (introIdx == 10) {  // Intro last has longer hold
            timer = holdLastFrames;
        } else {
            timer = holdFrames;
        }

        int textTimer = letterFrames;
        const char* textPointer = texts[introIdx];
        int initialX = 30;
        if (introIdx == 3 || introIdx == 6)  // Fit to screen
            initialX = 25;
        else if (introIdx == 5)  // MT EBOTT. centered
            initialX = 256 / 2 - 43;
        int x = initialX, y = 30;
        Engine::textSub.clear();
        while (timer >= 0 && !skip) {
            skip = keysDown() != 0;
            if (textTimer == 0 && *textPointer != 0) {
                char glyph = *textPointer++;
                if (glyph == '\n') {
                    x = initialX;
                    y += 20;
                } else {
                    Engine::textSub.drawGlyph(mainFont, glyph, x, y);
                    x += 3;
                    if (introIdx == 3)  // Fit to screen
                        x -= 1;
                }
                textTimer = letterFrames;
                if (glyph == '.' && introIdx != 5)  // Don't want time on MT. EBOTT
                    textTimer = 40;
                else if (glyph == ',' || glyph == ':' || glyph == ';')
                    textTimer = 20;
            }
            Engine::tick();
            timer--;
            if (*textPointer != 0)
                textTimer--;
        }

        if (introIdx == 10) {  // Intro last scroll
            timer = scrollFrames;
            while (timer >= 0 && !skip) {
                skip = keysDown() != 0;
                REG_BG3VOFS = ((height-192) * timer) / scrollFrames;
                Engine::tick();
                timer--;
            }

            timer = holdLastFrames;
            while (timer >= 0 && !skip) {
                skip = keysDown() != 0;
                Engine::tick();
                timer--;
            }
        }

        if (introIdx == 10)  // Intro last has longer fade out
            fadeOutFrames *= 5;  // Can change value as it's never used again
        timer = fadeOutFrames;
        while (timer >= 0 && !skip) {
            skip = keysDown() != 0;
            setBrightness(1, (-16 * (fadeOutFrames - timer)) / fadeOutFrames);
            if (introIdx == 6)
                setBrightness(2, (-16 * (fadeOutFrames - timer)) / fadeOutFrames);
            Engine::tick();
            timer--;
        }
    }
    REG_BG3VOFS = 0;
    Engine::textSub.clear();
    BGM::stopWAV();
    music.free_();

    BGM::playWAV(introSound);

    f = fopen("bg/intro/title.cbgf", "rb");
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

    Engine::loadBgMain(currentBackground);
    setBrightness(3, 0);  // set brightness to full bright

    timer = introLogoFrames;
    while (!skip) {
        skip = keysDown() != 0;
        Engine::tick();
        if (timer > 0) {
            timer--;
            if (timer <= 0) {
                const char* textPointer = pressText;
                int x = 20, y = 60;
                while (*textPointer != 0) {
                    Engine::textSub.drawGlyph(mainFont, *textPointer, x, y);
                    textPointer++;
                }
            }
        }
    }
    BGM::stopWAV();
    introSound.free_();
}
