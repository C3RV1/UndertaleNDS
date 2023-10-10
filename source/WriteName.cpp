//
// Created by cervi on 24/08/2022.
//

#include "WriteName.hpp"

#include "Engine/Background.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Font.hpp"
#include "Save.hpp"
#include "Formats/utils.hpp"


void runWriteNameMenu() {
    // letter selection
    constexpr int startX = 30, startY = 30, spacingX = 30, spacingY = 17;

    constexpr int nameX = 80, nameY = 80;
    constexpr int line1x = 55, line1y = 40;
    const int line2x = 50, line2y = 140;
    const int line3x = 65, line3y = 160;

    const int letterCount = 26 * 2;
    const int maxLen = MAX_NAME_LEN;

    Audio2::playBGMusic("mus_menu0.wav", true);

    Engine::clearMain();

    Engine::Font mainFont;
    mainFont.loadPath("fnt_maintext.font");

    FILE* textStream = fopen("nitro:/data/write_name.txt", "rb");
    if (textStream == nullptr)
        nocashMessage("Error opening write name text file.");
    else {
        char charBuffer;

        Engine::textMain.clear();

        fread(&charBuffer, 1, 1, textStream);
        int x = line1x, y = line1y;
        while (charBuffer != '\n') {
            Engine::textMain.drawGlyph(mainFont, charBuffer, x, y);
            fread(&charBuffer, 1, 1, textStream);
        }

        fread(&charBuffer, 1, 1, textStream);
        x = line2x; y = line2y;
        while (charBuffer != '\n') {
            Engine::textMain.drawGlyph(mainFont, charBuffer, x, y);
            fread(&charBuffer, 1, 1, textStream);
        }

        fread(&charBuffer, 1, 1, textStream);
        x = line3x; y = line3y;
        while (charBuffer != '\n') {
            Engine::textMain.drawGlyph(mainFont, charBuffer, x, y);
            fread(&charBuffer, 1, 1, textStream);
        }
    }

    char confirmText[100];
    int len = str_len_file(textStream, '\n');
    fread(confirmText, len + 1, 1, textStream);
    confirmText[len] = '\0';

    char confirmText2[100];
    len = str_len_file(textStream, '\n');
    fread(confirmText2, len + 1, 1, textStream);
    confirmText2[len] = '\0';

    char confirmText3[100];
    len = str_len_file(textStream, '\n');
    fread(confirmText3, len + 1, 1, textStream);
    confirmText3[len] = '\0';

    char confirmText4[100];
    len = str_len_file(textStream, '\n');
    fread(confirmText4, len + 1, 1, textStream);
    confirmText4[len] = '\0';

    fclose(textStream);

    char currentName[maxLen + 1] = {0};
    int currentLen = 0;

    int x, y;

    bool running = true;
    while (running) {
        int cLetter = 0;
        Engine::textSub.clear();
        for (char c = 'A', i = 0; c <= 'Z'; c++, i++) {
            x = i % 7;
            y = i / 7;
            x = startX + x * spacingX;
            y = startY + y * spacingY;
            if (i == 0)
                Engine::textSub.setColor(12);
            else
                Engine::textSub.setColor(15);

            Engine::textSub.drawGlyph(mainFont, c, x, y);
        }

        for (char c = 'a', i = 0; c <= 'z'; c++, i++) {
            x = i % 7;
            y = 4 + i / 7;
            x = startX + x * spacingX;
            y = startY + y * spacingY;

            Engine::textSub.drawGlyph(mainFont, c, x, y);
        }

        // get char loop
        for (;;) {
            Engine::tick();
            bool changed = false;
            int prevCLetter = cLetter;
            bool confirm = false;

            if (keysDown() & KEY_RIGHT && cLetter < letterCount - 1) {
                cLetter++;
                changed = true;
            } else if (keysDown() & KEY_LEFT && cLetter > 0) {
                cLetter--;
                changed = true;
            } else if (keysDown() & KEY_DOWN) {
                changed = true;
                if (cLetter <= 18) {
                    cLetter += 7;
                } else if (cLetter <= 20) {
                    cLetter += 7 + 5;
                } else if (cLetter <= 25) {
                    cLetter += 5;
                } else if (cLetter <= 26 + 18) {
                    cLetter += 7;
                }
            } else if (keysDown() & KEY_UP) {
                changed = true;
                if (cLetter >= 26 + 7) {
                    cLetter -= 7;
                } else if (cLetter >= 26 + 5) {
                    cLetter -= 7 + 5;
                } else if (cLetter >= 26) {
                    cLetter -= 5;
                } else if (cLetter >= 7) {
                    cLetter -= 7;
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
                            cLetter = tileIdx;
                            changed = true;
                        }
                    }
                }
                confirm = changed;
            }

            if (confirm || keysDown() & KEY_A) {
                if (cLetter < letterCount && currentLen <= maxLen) {
                    // letter
                    char glyph;
                    if (cLetter < 26) {
                        glyph = (char) cLetter + 'A';
                    } else {
                        glyph = (char) (cLetter - 26) + 'a';
                    }
                    currentName[currentLen] = glyph;
                    currentLen++;
                    x = nameX;
                    y = nameY;
                    for (char *src = currentName; src <= currentName + currentLen; src++) {
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
                            Engine::textMain.setColor(0); // transparent to clear char
                        Engine::textMain.drawGlyph(mainFont, *src, x, y);
                    }
                    currentName[currentLen - 1] = 0;
                    currentLen--;
                    Engine::textMain.setColor(15);  // white
                }
            }

            if (changed) {
                for (int i = 0; i < 2; i++) {
                    int letterToChange;
                    int colorToChangeTo;
                    if (i == 0) {
                        letterToChange = prevCLetter;
                        colorToChangeTo = 15;
                    } else {
                        letterToChange = cLetter;
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
                    Engine::textSub.setColor(colorToChangeTo);
                    Engine::textSub.drawGlyph(mainFont, glyph, x, y);
                }
            }
        }

        // ask for confirmation
        // if confirmation, break
        // if not, paints again the letters to btm
        Engine::textSub.clear();
        Engine::textSub.setColor(15);
        x = 30; y = 30;
        for (const char* t = confirmText; *t != 0; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        Engine::textSub.setColor(12);
        for (char *t = currentName; t < currentName + currentLen; t++) {
            Engine::textSub.drawGlyph(mainFont, *t, x, y);
        }
        Engine::textSub.setColor(15);
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
    Audio2::stopBGMusic();

    memset(globalSave.name, 0, currentLen + 1);
    memcpy(globalSave.name, currentName, currentLen + 1);
    std::string buffer = "Selected name: " + std::string(globalSave.name);
    nocashMessage(buffer.c_str());
}
