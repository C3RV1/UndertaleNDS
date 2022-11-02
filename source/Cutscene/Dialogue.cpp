//
// Created by cervi on 31/08/2022.
//

#include "Cutscene/Dialogue.hpp"
#include "Engine/OAMManager.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Formats/utils.hpp"

Dialogue::Dialogue(bool centered_, u16 textId, const char* speaker, s32 speakerX, s32 speakerY,
                   const char* idleAnimTxt, const char* talkAnimTxt, Engine::Sprite* target_,
                   const char* idleAnim2Txt, const char* talkAnim2Txt, const char* typeSndPath,
                   const char* fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager) :
                   speakerManager(Engine::AllocatedOAM) {
    centered = centered_;
    textManager = &txtManager;
    char buffer[100];

    font.loadPath(fontTxt);

    if (strlen(speaker) != 0 && centered_)
        speakerSpr.loadPath(speaker);

    sprintf(buffer, "nitro:/data/dialogue/r%d/c%d/d%d.txt",
            globalCutscene->roomId, globalCutscene->cutsceneId, textId);
    FILE* textStream = fopen(buffer, "rb");
    fseek(textStream, 0, SEEK_END);
    textLen = ftell(textStream);
    fseek(textStream, 0, SEEK_SET);
    text = new char[textLen];
    fread(text, textLen, 1, textStream);
    if (centered_) {
        if (strlen(speaker) != 0)
            startingY = 192 / 2;
        else
            startingY = 192 / 4;
        y = startingY;
    }
    else {
        x = speakerX >> 8;
        startingX = x;
        y = speakerY >> 8;
        startingY = y;
    }
    letterFrames = framesPerLetter;
    currentTimer = letterFrames;
    if (centered_) {
        getLine();

        speakerManager.loadTexture(speakerSpr);
        speakerManager.wx = speakerX;
        speakerManager.wy = speakerY;
        speakerManager.setShown(true);
        idleAnim = speakerManager.nameToAnimId(idleAnimTxt);
        talkAnim = speakerManager.nameToAnimId(talkAnimTxt);
    }

    target = target_;
    if (target_ != nullptr) {
        idleAnim2 = target_->nameToAnimId(idleAnim2Txt);
        talkAnim2 = target_->nameToAnimId(talkAnim2Txt);
    }

    textManager->clear();
    textManager->reloadColors();
    textManager->setCurrentColor(15);

    typeSnd.loadWAV(typeSndPath);
    typeSnd.setLoops(0);

    setTalk();
}

Dialogue::Dialogue(bool centered_, int x_, int y_, const char *text_, const char *typeSndPath,
                   const char *fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager) :
                   speakerManager(Engine::AllocatedOAM) {
    centered = centered_;
    textManager = &txtManager;
    font.loadPath(fontTxt);
    textLen = strlen(text_);
    text = new char[textLen];
    strcpy(text, text_);
    typeSnd.loadWAV(typeSndPath);
    typeSnd.setLoops(0);
    letterFrames = framesPerLetter;
    currentTimer = letterFrames;
    target = nullptr;
    if (centered_) {
        startingY = 192 / 4;
        y = startingY;
    }
    else {
        x = x_;
        y = y_;
        startingX = x;
        startingY = y;
    }

    setTalk();
}

void Dialogue::setTalk() {
    speakerManager.setSpriteAnim(talkAnim);
    if (target != nullptr)
        target->setSpriteAnim(talkAnim2);
}

void Dialogue::setNoTalk() {
    speakerManager.setSpriteAnim(idleAnim);
    if (target != nullptr)
        target->setSpriteAnim(idleAnim2);
}

bool Dialogue::update() {
    if (!paused) {
        setTalk();
        progressText(true, true);
        if (((keysDown() & (/*KEY_TOUCH |*/ KEY_B)) || letterFrames == 0) && (textPos != textLen)) {
            progressText(true, false);
            while (!paused && !(textPos >= textLen && (linePos >= lineLen || !centered)))
                progressText(false, false);
            if (centered && linePos > 0) {
                linePos--;
                progressText(false, true);
            }
        }
        if (textPos == textLen && (linePos >= lineLen || !centered)) {
            setNoTalk();
            return true;
        }
        return false;
    } else {
        setNoTalk();
        if (keysDown() & (/*KEY_TOUCH |*/ KEY_A)) {
            paused = false;
            progressText(true, true);
            return false;
        }
    }
    return false;
}

u16 Dialogue::getLineWidth(int linePos_) {
    u16 lineWidth_ = 0;
    for (char* pLine = line; pLine < line + linePos_; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        lineWidth_ += font.getGlyphWidth(*pLine);
        lineWidth_++;
    }
    return lineWidth_ - 1;
}

void Dialogue::getLine() {
    if (text == nullptr)
        return;
    for (lineLen = 0; *(text + textPos + lineLen) != '\n'; lineLen++);
    memcpy(line, text + textPos, lineLen + 1);
    line[lineLen] = '\0';
    textPos += lineLen + 1;
}

void Dialogue::progressText(bool clear, bool draw) {
    if (centered)
        progressTextCentered(clear, draw);
    else
        progressTextLeft(clear, draw);
}

void Dialogue::progressTextCentered(bool clear, bool draw) {
    if (currentTimer > 0 && draw) {
        currentTimer--;
        return;
    }
    currentTimer = letterFrames;
    if (linePos == lineLen) {
        if (textPos >= textLen) {
            return;
        }
        y += lineSpacing;
        linePos = 0;
        getLine();
    }
    if (*(line + linePos) == '@') {
        linePos++;
        char command = *(line + linePos);
        linePos++;
        if (command == 'p') {
            paused = true;
        }
        else if (command == 'c') {
            textManager->clear();
            y = startingY;
        }
        else if (command == 'a') {
            char buffer[30];
            char* pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != '/';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            idleAnim = speakerManager.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != '/';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            talkAnim = speakerManager.nameToAnimId(buffer);
        }
        else if (command == 'b') {
            char buffer[30];
            char* pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != '/';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            idleAnim2 = speakerManager.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != '/';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            talkAnim2 = speakerManager.nameToAnimId(buffer);
        }
        currentTimer = 0;
        return;
    }
    linePos++;
    if (draw && linePos < lineLen)
        typeSnd.play();

    // clear current chars
    u16 width = getLineWidth(linePos - 1);
    startingX = 128 - width / 2;
    textManager->setCurrentColor(0); // clear color
    for (char* pLine = line; pLine < line + linePos - 1; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        if (clear || linePos > lineLen)
            textManager->drawGlyph(font, *pLine, startingX, y);
        startingX += 1;
    }

    width = getLineWidth(linePos);
    startingX = 128 - width / 2;
    textManager->setCurrentColor(currentColor); // clear color
    int lineEndColor = currentColor;
    for (char* pLine = line; pLine < line + linePos; pLine++) {
        if (*pLine == '@') {
            pLine++;
            char command = *pLine;
            if (command == '0')
                textManager->setCurrentColor(8);
            else if (command == '1')
                textManager->setCurrentColor(9);
            else if (command == '2')
                textManager->setCurrentColor(10);
            else if (command == '3')
                textManager->setCurrentColor(11);
            else if (command == '4')
                textManager->setCurrentColor(12);
            else if (command == '5')
                textManager->setCurrentColor(13);
            else if (command == '6')
                textManager->setCurrentColor(14);
            else if (command == 'w')
                textManager->setCurrentColor(15);
            lineEndColor = textManager->getCurrentColor();
            continue;
        }
        if (draw || linePos == lineLen)
            textManager->drawGlyph(font, *pLine, startingX, y);
        startingX += 1;
    }

    if (linePos == lineLen)
        currentColor = lineEndColor;
}

void Dialogue::progressTextLeft(bool, bool draw) {
    if (currentTimer > 0 && draw) {
        currentTimer--;
        return;
    }
    currentTimer = letterFrames;
    if (textPos >= textLen) {
        return;
    }

    char currentChar;
    if (text != nullptr)
        currentChar = text[textPos++];
    else
        return;

    if (currentChar == '@') {
        currentChar = text[textPos++];
        if (currentChar == 'p') {
            paused = true;
        }
        else if (currentChar == 'c') {
            textManager->clear();
            x = startingX;
            y = startingY;
        }
        else if (currentChar == '0')
            textManager->setCurrentColor(8);
        else if (currentChar == '1')
            textManager->setCurrentColor(9);
        else if (currentChar == '2')
            textManager->setCurrentColor(10);
        else if (currentChar == '3')
            textManager->setCurrentColor(11);
        else if (currentChar == '4')
            textManager->setCurrentColor(12);
        else if (currentChar == '5')
            textManager->setCurrentColor(13);
        else if (currentChar == '6')
            textManager->setCurrentColor(14);
        else if (currentChar == 'w')
            textManager->setCurrentColor(15);
        else if (currentChar == 'a') {
            int len;
            for (len = 0; *(text + textPos + len) != '/'; len++);
            char buffer[30];
            memcpy(buffer, text + textPos, len + 1);
            buffer[len] = '\0';
            textPos += len + 1;
            idleAnim = speakerManager.nameToAnimId(buffer);
            for (len = 0; *(text + textPos + len) != '/'; len++);
            memcpy(buffer, text + textPos, len + 1);
            buffer[len] = '\0';
            textPos += len + 1;
            talkAnim = speakerManager.nameToAnimId(buffer);
        }
        else if (currentChar == 'b') {
            int len;
            for (len = 0; *(text + textPos + len) != '/'; len++);
            char buffer[30];
            memcpy(buffer, text + textPos, len + 1);
            buffer[len] = '\0';
            textPos += len + 1;
            if (target != nullptr)
                idleAnim2 = target->nameToAnimId(buffer);
            for (len = 0; *(text + textPos + len) != '/'; len++);
            memcpy(buffer, text + textPos, len + 1);
            buffer[len] = '\0';
            textPos += len + 1;
            if (target != nullptr)
                talkAnim2 = target->nameToAnimId(buffer);
        }
        return;
    } else if (currentChar == '\n') {
        y += lineSpacing;
        x = startingX;
        return;
    }
    if (draw)
        typeSnd.play();

    textManager->drawGlyph(font, currentChar, x, y);
}

void Dialogue::free_() {
    speakerManager.setShown(false);
    speakerSpr.free_();
    typeSnd.stop();
    typeSnd.free_();
}
