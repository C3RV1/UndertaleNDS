//
// Created by cervi on 31/08/2022.
//

#include "Cutscene/Dialogue.hpp"

Dialogue::Dialogue(bool isRoom_, uint16_t textId, char *speaker, int32_t speakerX, int32_t speakerY,
                   char *idleAnimTxt, char *talkAnimTxt,
                   Engine::Sprite *target_, char *idleAnim2Txt, char *talkAnim2Txt,
                   char* fontTxt, uint16_t framesPerLetter) :
                   speakerManager(Engine::AllocatedOAM) {
    isRoom = isRoom_;
    char buffer[100];

    sprintf(buffer, "nitro:/fnt/%s", fontTxt);
    FILE *fontFile = fopen(buffer, "rb");
    if (fontFile) {
        int fntLoad = font.loadCFNT(fontFile);
        if (fntLoad != 0) {
            sprintf(buffer, "Error loading font %s: %d", speaker,
                    fntLoad);
            nocashMessage(buffer);
        }
    } else {
        sprintf(buffer, "Error opening font %s", fontTxt);
        nocashMessage(buffer);
    }
    fclose(fontFile);

    if (strlen(speaker) != 0 && isRoom_) {
        sprintf(buffer, "nitro:/spr/%s", speaker);
        FILE *speakerCspr = fopen(buffer, "rb");
        if (speakerCspr) {
            int sprLoad = speakerSpr.loadCSPR(speakerCspr);
            if (sprLoad != 0) {
                sprintf(buffer, "Error loading speaker %s: %d", speaker,
                        sprLoad);
                nocashMessage(buffer);
            }
        } else {
            sprintf(buffer, "Error opening speaker %s", speaker);
            nocashMessage(buffer);
        }
        fclose(speakerCspr);
    }

    sprintf(buffer, "nitro:/data/dialogue/dialogue_%d_%d.txt",
            globalCutscene->cutsceneId, textId);
    textStream = fopen(buffer, "rb");
    fseek(textStream, 0, SEEK_END);
    textLen = ftell(textStream);
    fseek(textStream, 0, SEEK_SET);
    if (isRoom_) {
        startingY = 192 / 2;
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
    if (isRoom_) {
        getLine();

        speakerManager.loadSprite(speakerSpr);
        speakerManager.wx = speakerX;
        speakerManager.wy = speakerY;
        speakerManager.setShown(true);
        idleAnim = speakerManager.nameToAnimId(idleAnimTxt);
        talkAnim = speakerManager.nameToAnimId(talkAnimTxt);
    }

    target = target_;
    idleAnim2 = target_->nameToAnimId(idleAnim2Txt);
    talkAnim2 = target_->nameToAnimId(talkAnim2Txt);

    Engine::textSub.clear();
    Engine::textSub.reloadColors();
    Engine::textSub.setCurrentColor(15);
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
        if ((keysDown() & (KEY_TOUCH | KEY_A)) || letterFrames == 0) {
            progressText(true, false);
            while (!paused && !(ftell(textStream) == textLen && (linePos > lineLen || !isRoom)))
                progressText(false, false);
            linePos--;
            progressText(false, true);
        }
        if (ftell(textStream) == textLen && (linePos > lineLen || !isRoom)) {
            setNoTalk();
            fclose(textStream);
            return true;
        }
        return false;
    } else {
        setNoTalk();
        if (keysDown() & (KEY_TOUCH | KEY_A)) {
            paused = false;
            progressText(true, true);
            return false;
        }
    }
    return false;
}

uint16_t Dialogue::getLineWidth(int linePos_) {
    uint16_t lineWidth_ = 0;
    for (char* pLine = line; pLine < line + linePos_; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        lineWidth_ += Engine::textSub.getGlyphWidth(font, *pLine);
        lineWidth_++;
    }
    return lineWidth_ - 1;
}

void Dialogue::getLine() {
    if (textStream == nullptr)
        return;
    lineLen = strlen_file(textStream, '\n');
    fread(line, lineLen + 1, 1, textStream);
}

void Dialogue::progressText(bool clear, bool draw) {
    if (isRoom)
        progressTextRoom(clear, draw);
    else
        progressTextBattle(clear, draw);
}

void Dialogue::progressTextRoom(bool clear, bool draw) {
    if (currentTimer > 0 && draw) {
        currentTimer--;
        return;
    }
    currentTimer = letterFrames;
    if (linePos > lineLen) {
        if (ftell(textStream) == textLen) {
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
            Engine::textSub.clear();
            y = startingY;
        }
        else if (command == 'a') {
            char buffer[30];
            char* pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != ' ';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            idleAnim = speakerManager.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != ' ';
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
            for (char* pLine = (line + linePos); *pLine != ' ';
                 pLine++, pBuffer++, linePos++) {
                *pBuffer = *pLine;
            }
            linePos++;
            *pBuffer = '\0';
            idleAnim2 = speakerManager.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (line + linePos); *pLine != ' ';
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

    // clear current chars
    uint16_t width = getLineWidth(linePos - 1);
    startingX = 128 - width / 2;
    Engine::textSub.setCurrentColor(0); // clear color
    for (char* pLine = line; pLine < line + linePos - 1; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        if (clear || linePos > lineLen)
            Engine::textSub.drawGlyph(font, *pLine, startingX, y);
        startingX += 1;
    }

    width = getLineWidth(linePos);
    startingX = 128 - width / 2;
    Engine::textSub.setCurrentColor(currentColor); // clear color
    int lineEndColor = currentColor;
    for (char* pLine = line; pLine < line + linePos; pLine++) {
        if (*pLine == '@') {
            pLine++;
            char command = *pLine;
            if (command == '0')
                Engine::textSub.setCurrentColor(8);
            else if (command == '1')
                Engine::textSub.setCurrentColor(9);
            else if (command == '2')
                Engine::textSub.setCurrentColor(10);
            else if (command == '3')
                Engine::textSub.setCurrentColor(11);
            else if (command == '4')
                Engine::textSub.setCurrentColor(12);
            else if (command == '5')
                Engine::textSub.setCurrentColor(13);
            else if (command == '6')
                Engine::textSub.setCurrentColor(14);
            else if (command == 'w')
                Engine::textSub.setCurrentColor(15);
            lineEndColor = Engine::textSub.getCurrentColor();
            continue;
        }
        if (draw || linePos > lineLen)
            Engine::textSub.drawGlyph(font, *pLine, startingX, y);
        startingX += 1;
    }

    if (linePos > lineLen)
        currentColor = lineEndColor;
}

void Dialogue::progressTextBattle(bool clear, bool draw) {
    if (currentTimer > 0 && draw) {
        currentTimer--;
        return;
    }
    currentTimer = letterFrames;
    if (linePos == textLen) {
        return;
    }

    char currentChar;
    fread(&currentChar, 1, 1, textStream);
    linePos++;
    if (currentChar == '@') {
        fread(&currentChar, 1, 1, textStream);  // read command
        linePos++;
        if (currentChar == 'p') {
            paused = true;
        }
        else if (currentChar == 'c') {
            Engine::textSub.clear();
            x = startingX;
            y = startingY;
        }
        else if (currentChar == '0')
            Engine::textSub.setCurrentColor(8);
        else if (currentChar == '1')
            Engine::textSub.setCurrentColor(9);
        else if (currentChar == '2')
            Engine::textSub.setCurrentColor(10);
        else if (currentChar == '3')
            Engine::textSub.setCurrentColor(11);
        else if (currentChar == '4')
            Engine::textSub.setCurrentColor(12);
        else if (currentChar == '5')
            Engine::textSub.setCurrentColor(13);
        else if (currentChar == '0')
            Engine::textSub.setCurrentColor(14);
        else if (currentChar == 'w')
            Engine::textSub.setCurrentColor(15);
        else if (currentChar == 'a') {
            int len = strlen_file(textStream, ' ');
            char buffer[30];
            fread(buffer, len + 1, 1, textStream);
            linePos += len + 1;
            buffer[len] = '\0';
            idleAnim = speakerManager.nameToAnimId(buffer);
            len = strlen_file(textStream, ' ');
            fread(buffer, len + 1, 1, textStream);
            buffer[len] = '\0';
            talkAnim = speakerManager.nameToAnimId(buffer);
        }
        else if (currentChar == 'b') {
            int len = strlen_file(textStream, ' ');
            char buffer[30];
            fread(buffer, len + 1, 1, textStream);
            linePos += len + 1;
            buffer[len] = '\0';
            if (target != nullptr)
                idleAnim2 = target->nameToAnimId(buffer);
            len = strlen_file(textStream, ' ');
            fread(buffer, len + 1, 1, textStream);
            buffer[len] = '\0';
            if (target != nullptr)
                talkAnim2 = target->nameToAnimId(buffer);
        }
        return;
    } else if (currentChar == '\n') {
        y += lineSpacing;
        x = startingX;
        return;
    }

    Engine::textSub.drawGlyph(font, currentChar, x, y);
}

void Dialogue::free_() {
    speakerManager.setShown(false);
    speakerSpr.free_();
}

Dialogue* currentDialogue = nullptr;
