//
// Created by cervi on 31/08/2022.
//

#include "Cutscene/Dialogue.hpp"

Dialogue::Dialogue(uint16_t textId, char *speaker, int32_t speakerX, int32_t speakerY,
                   char *idleAnimTxt, char *talkAnimTxt,
                   Engine::SpriteManager *target_, char *idleAnim2Txt, char *talkAnim2Txt,
                   char* fontTxt, uint16_t framesPerLetter) :
                   speakerManager(Engine::AllocatedOAM) {
    char buffer[100];
    if (strlen(speaker) != 0) {
        FILE *speakerCspr = fopen(speaker, "rb");
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

    FILE *fontFile = fopen(fontTxt, "rb");
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

    sprintf(buffer, "nitro:/data/dialogue/dialogue_%d_%d.txt",
            globalCutscene->cutsceneId, textId);
    textStream = fopen(buffer, "rb");
    fseek(textStream, 0, SEEK_END);
    textLen = ftell(textStream);
    fseek(textStream, 0, SEEK_SET);
    y = startingY;
    letterFrames = framesPerLetter;
    currentTimer = letterFrames;
    getLine();

    speakerManager.loadSprite(speakerSpr);
    speakerManager.wx = speakerX;
    speakerManager.wy = speakerY;
    speakerManager.setShown(true);
    idleAnim = speakerManager.nameToAnimId(idleAnimTxt);
    talkAnim = speakerManager.nameToAnimId(talkAnimTxt);

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
        progressText();
        return false;
    } else {
        setNoTalk();
        if (keysDown() & (KEY_TOUCH | KEY_A)) {
            paused = false;
            if (ftell(textStream) == textLen) {
                return true;
            }
            progressText();
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

void Dialogue::progressText() {
    if (currentTimer > 0) {
        currentTimer--;
        return;
    }
    currentTimer = letterFrames;
    if (linePos > lineLen) {
        if (ftell(textStream) == textLen) {
            paused = true;
            return;
        }
        y += lineSpacing;
        linePos = 0;
        getLine();
        char buffer[100];
        sprintf(buffer, "line len %d", lineLen);
        nocashMessage(buffer);
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
        currentTimer = 0;
        return;
    }
    linePos++;

    // clear current chars
    uint16_t width = getLineWidth(linePos - 1);
    int startingX = 128 - width / 2;
    Engine::textSub.setCurrentColor(0); // clear color
    for (char* pLine = line; pLine < line + linePos - 1; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
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
                Engine::textSub.setCurrentColor(9);
            else if (command == '1')
                Engine::textSub.setCurrentColor(10);
            else if (command == '2')
                Engine::textSub.setCurrentColor(11);
            else if (command == '3')
                Engine::textSub.setCurrentColor(12);
            else if (command == '4')
                Engine::textSub.setCurrentColor(13);
            else if (command == '5')
                Engine::textSub.setCurrentColor(14);
            else if (command == 'w')
                Engine::textSub.setCurrentColor(15);
            lineEndColor = Engine::textSub.getCurrentColor();
            continue;
        }
        Engine::textSub.drawGlyph(font, *pLine, startingX, y);
        startingX += 1;
    }

    if (linePos > lineLen)
        currentColor = lineEndColor;
}

void Dialogue::free_() {
    speakerManager.setShown(false);
    speakerSpr.free_();
    Engine::textSub.clear();
}

Dialogue* currentDialogue = nullptr;
