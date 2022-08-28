//
// Created by cervi on 24/08/2022.
//

#include "WriteName.hpp"


void writeNameMenu() {
    char buffer[100];

    BGM::WAV music;
    int loadMusic = music.loadWAV("nitro:/z_audio/mus_menu0.wav");
    sprintf(buffer, "Load mus_menu0.wav: %d", loadMusic);
    BGM::playWAV(music);

    const int startX = 30, startY = 30, spacingX = 30, spacingY = 17;
    const int nameX = 80, nameY = 80;

    Engine::clearMain();

    Engine::Font mainFont;
    FILE *f;
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

    const char* topText = "Name the fallen human.";
    const char* topText2 = "Press START to confirm.";
    const char* topText3 = "Press B to delete.";
    int x = 55, y = 40;
    Engine::textMain.clear();
    while (*topText != 0) {
        Engine::textMain.drawGlyph(mainFont, *topText++, x, y);
    }
    x = 50; y = 140;
    while (*topText2 != 0) {
        Engine::textMain.drawGlyph(mainFont, *topText2++, x, y);
    }
    x = 65; y = 160;
    while (*topText3 != 0) {
        Engine::textMain.drawGlyph(mainFont, *topText3++, x, y);
    }

    const char* confirmText = "Is \"";
    const char* confirmText2 = "\" correct?";
    const char* confirmText3 = "START to confirm";
    const char* confirmText4 = "B to go back";

    const int letterCount = 26 * 2;

    char currentName[10] = {0};
    int currentLen = 0;
    const int maxLen = 10;

    bool running = true;
    while (running) {
        int currentLetter = 0;
        Engine::textSub.clear();
        Engine::textSub.setPaletteColor(12, 255, 255, 0, true);
        for (char c = 'A', i = 0; c <= 'Z'; c++, i++) {
            x = i % 7;
            y = i / 7;
            x = startX + x * spacingX;
            y = startY + y * spacingY;
            if (i == 0)
                Engine::textSub.setCurrentColor(12);
            else
                Engine::textSub.setCurrentColor(15);

            Engine::textSub.drawGlyph(mainFont, c, x, y);
        }

        for (char c = 'a', i = 0; c <= 'z'; c++, i++) {
            x = i % 7;
            y = 4 + i / 7;
            x = startX + x * spacingX;
            y = startY + y * spacingY;

            Engine::textSub.drawGlyph(mainFont, c, x, y);
        }

        // get char rutine
        for (;;) {
            Engine::tick();
            bool changed = false;
            int prevCurrentLetter = currentLetter;
            bool confirm = false;

            if (keysDown() & KEY_RIGHT && currentLetter < letterCount - 1) {
                currentLetter++;
                changed = true;
            } else if (keysDown() & KEY_LEFT && currentLetter > 0) {
                currentLetter--;
                changed = true;
            } else if (keysDown() & KEY_DOWN) {
                changed = true;
                if (currentLetter <= 18) {
                    currentLetter += 7;
                } else if (currentLetter <= 20) {
                    currentLetter += 7 + 5;
                } else if (currentLetter <= 25) {
                    currentLetter += 5;
                } else if (currentLetter <= 26 + 18) {
                    currentLetter += 7;
                }
            } else if (keysDown() & KEY_UP) {
                changed = true;
                if (currentLetter >= 26 + 7) {
                    currentLetter -= 7;
                } else if (currentLetter >= 26 + 5) {
                    currentLetter -= 7 + 5;
                } else if (currentLetter >= 26) {
                    currentLetter -= 5;
                } else if (currentLetter >= 7) {
                    currentLetter -= 7;
                }
            } else if (keysDown() & KEY_TOUCH) {
                touchPosition touch;
                touchRead(&touch);
                if (touch.px >= startX - spacingX / 2 && touch.py >= startY) {
                    touch.px -= startX - spacingX / 2;
                    touch.py -= startY;
                    if (touch.px <= 7 * spacingX && touch.py <= 8 * spacingY) {
                        int tileX = touch.px / spacingX;
                        int tileY = touch.py / spacingY;
                        int tileIdx = tileY * 7 + tileX;
                        if (tileIdx < 26 || tileIdx >= 28) {
                            if (tileIdx >= 28) {
                                tileIdx -= 2;
                            }
                            currentLetter = tileIdx;
                            changed = true;
                        }
                    }
                }
                confirm = changed;
            }

            if (confirm || keysDown() & KEY_A) {
                if (currentLetter < letterCount && currentLen < maxLen) {
                    // letter
                    char glyph;
                    if (currentLetter < 26) {
                        glyph = (char) currentLetter + 'A';
                    } else {
                        glyph = (char) (currentLetter - 26) + 'a';
                    }
                    currentLen++;
                    currentName[currentLen - 1] = glyph;
                    x = nameX;
                    y = nameY;
                    for (char *src = currentName; src < currentName + currentLen; src++) {
                        Engine::textMain.drawGlyph(mainFont, *src, x, y);
                    }
                }
            }

            if (keysDown() & KEY_START) {
                break;
            }

            if (keysDown() & KEY_B) {
                if (currentLen > 0) {
                    x = nameX;
                    y = nameY;
                    for (char *src = currentName; src < currentName + currentLen; src++) {
                        if (src == currentName + currentLen - 1)
                            Engine::textMain.setCurrentColor(0); // transparent to clear char
                        Engine::textMain.drawGlyph(mainFont, *src, x, y);
                    }
                    currentName[currentLen - 1] = 0;
                    currentLen--;
                    Engine::textMain.setCurrentColor(15);  // white
                }
            }

            if (changed) {
                for (int i = 0; i < 2; i++) {
                    int letterToChange;
                    int colorToChangeTo;
                    if (i == 0) {
                        letterToChange = prevCurrentLetter;
                        colorToChangeTo = 15;
                    } else {
                        letterToChange = currentLetter;
                        colorToChangeTo = 12;
                    }
                    char glyph;
                    if (letterToChange < 26) {
                        x = letterToChange % 7;
                        y = letterToChange / 7;
                        glyph = (char) letterToChange + 'A';
                    } else {
                        letterToChange -= 26;
                        x = letterToChange % 7;
                        y = 4 + letterToChange / 7;
                        glyph = (char) letterToChange + 'a';
                    }
                    x = startX + x * spacingX;
                    y = startY + y * spacingY;
                    Engine::textSub.setCurrentColor(colorToChangeTo);
                    Engine::textSub.drawGlyph(mainFont, glyph, x, y);
                }
            }
        }

        // ask for confirmation
        // if confirmation, break
        // if not, paints again the letters to btm
        Engine::textSub.clear();
        Engine::textSub.setCurrentColor(15);
        x = 30; y = 30;
        for (const char* t = confirmText; *t != 0; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        Engine::textSub.setCurrentColor(12);
        for (char *t = currentName; t < currentName + currentLen; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        Engine::textSub.setCurrentColor(15);
        for (const char* t = confirmText2; *t != 0; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        x = 30; y = 60;
        for (const char* t = confirmText3; *t != 0; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        x = 30; y = 80;
        for (const char* t = confirmText4; *t != 0; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }

        for(;;) {
            Engine::tick();
            if (keysDown() & KEY_START) {
                running = false;
                break;
            } else if (keysDown() & KEY_B) {
                break;
            }
        }
    }
    BGM::stopWAV();

    if (saveGlobal.name != nullptr)
        free(saveGlobal.name);
    saveGlobal.name = (char*) malloc(currentLen + 1);
    memset(saveGlobal.name, 0, currentLen + 1);
    memcpy(saveGlobal.name, currentName, currentLen);
    sprintf(buffer, "Selected name: %s", saveGlobal.name);
    nocashMessage(buffer);
}
