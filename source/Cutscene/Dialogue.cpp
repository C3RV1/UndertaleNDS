//
// Created by cervi on 31/08/2022.
//

#include "Cutscene/Dialogue.hpp"
#include "Engine/OAMManager.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Formats/utils.hpp"

Dialogue::Dialogue(bool centered, u16 textId, const char* speaker, s32 speakerX, s32 speakerY,
                   const char* idleAnimTxt, const char* talkAnimTxt, Engine::Sprite* target,
                   const char* idleAnim2Txt, const char* talkAnim2Txt, const char* typeSndPath,
                   const char* fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager) :

        _centered(centered), _speakerSpr(Engine::AllocatedOAM),
        _target(target), _textManager(&txtManager) {
    char buffer[100];

    _fnt.loadPath(fontTxt);

    if (strlen(speaker) != 0 && centered)
        _speakerTex.loadPath(speaker);

    sprintf(buffer, "nitro:/data/dialogue/r%d/c%d/d%d.txt",
            globalCutscene->_roomId, globalCutscene->_cutsceneId, textId);
    FILE* textStream = fopen(buffer, "rb");
    fseek(textStream, 0, SEEK_END);
    _textLen = ftell(textStream);
    fseek(textStream, 0, SEEK_SET);
    _text = new char[_textLen];
    fread(_text, _textLen, 1, textStream);
    _textPos = _text;
    _lineStart = _text;
    _textEnd = _text + _textLen;
    if (centered) {
        if (strlen(speaker) != 0)
            _startingY = 192 / 2;
        else
            _startingY = 192 / 4;
        _y = _startingY;
    }
    else {
        _x = speakerX >> 8;
        _startingX = _x;
        _y = speakerY >> 8;
        _startingY = _y;
    }
    _letterFrames = framesPerLetter;
    _cTimer = _letterFrames;
    if (centered) {
        _speakerSpr.loadTexture(_speakerTex);
        _speakerSpr._wx = speakerX;
        _speakerSpr._wy = speakerY;
        _speakerSpr.setShown(true);
        _idleAnim = _speakerSpr.nameToAnimId(idleAnimTxt);
        _talkAnim = _speakerSpr.nameToAnimId(talkAnimTxt);
    }

    if (target != nullptr) {
        _idleAnim2 = target->nameToAnimId(idleAnim2Txt);
        _talkAnim2 = target->nameToAnimId(talkAnim2Txt);
    }

    _textManager->clear();
    _textManager->reloadColors();
    _textManager->setColor(15);

    _typeSnd.loadWAV(typeSndPath);
    _typeSnd.setLoops(0);

    setTalk();
}

Dialogue::Dialogue(bool centered_, int x_, int y_, const char *text_, const char *typeSndPath,
                   const char *fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager) :
        _speakerSpr(Engine::AllocatedOAM) {
    _centered = centered_;
    _textManager = &txtManager;
    _fnt.loadPath(fontTxt);
    _textLen = strlen(text_);
    _text = new char[_textLen];
    strcpy(_text, text_);
    _typeSnd.loadWAV(typeSndPath);
    _typeSnd.setLoops(0);
    _letterFrames = framesPerLetter;
    _cTimer = _letterFrames;
    _target = nullptr;
    if (centered_) {
        _startingY = 192 / 4;
        _y = _startingY;
    }
    else {
        _x = x_;
        _y = y_;
        _startingX = _x;
        _startingY = _y;
    }

    setTalk();
}

void Dialogue::setTalk() {
    _speakerSpr.setSpriteAnim(_talkAnim);
    if (_target != nullptr)
        _target->setSpriteAnim(_talkAnim2);
}

void Dialogue::setNoTalk() {
    _speakerSpr.setSpriteAnim(_idleAnim);
    if (_target != nullptr)
        _target->setSpriteAnim(_idleAnim2);
}

bool Dialogue::update() {
    if (!_paused) {
        setTalk();
        progressText(true, true);
        if (((keysDown() & (/*KEY_TOUCH |*/ KEY_B)) || _letterFrames == 0)) {
            progressText(true, false);
            while (!_paused && _textPos < _textEnd)
                progressText(false, false);
        }
        if (_textPos == _textEnd) {
            setNoTalk();
            return true;
        }
        return false;
    } else {
        setNoTalk();
        if (keysDown() & (/*KEY_TOUCH |*/ KEY_A)) {
            _paused = false;
            progressText(true, true);
            return false;
        }
    }
    return false;
}

u16 Dialogue::getLineWidth() {
    u16 lineWidth_ = 0;
    for (char* pLine = _lineStart; pLine < _textPos; pLine++) {
        if (*pLine == '@') {
            pLine++;
            char command = *pLine;
            if (command == 'a' or command == 'b') {
                for(;*pLine != '/';pLine++);
                pLine++;
                for(;*pLine != '/';pLine++);
            }
            continue;
        }
        lineWidth_ += _fnt.getGlyphWidth(*pLine);
        lineWidth_++;
    }
    return lineWidth_ - 1;
}

void Dialogue::progressText(bool clear, bool draw) {
    if (_cTimer > 0 && draw) {
        _cTimer--;
        return;
    }
    _cTimer = _letterFrames;

    char cChar = *_textPos++;
    if (cChar == '@') {
        cChar = *_textPos++;
        if (cChar == 'p') {
            _paused = true;
            if (_centered)
                drawTextCentered();
        }
        else if (cChar == 'c') {
            _textManager->clear();
            _x = _startingX;
            _y = _startingY;
        }
        else if (cChar == '0')
            _textManager->setColor(8);
        else if (cChar == '1')
            _textManager->setColor(9);
        else if (cChar == '2')
            _textManager->setColor(10);
        else if (cChar == '3')
            _textManager->setColor(11);
        else if (cChar == '4')
            _textManager->setColor(12);
        else if (cChar == '5')
            _textManager->setColor(13);
        else if (cChar == '6')
            _textManager->setColor(14);
        else if (cChar == 'w')
            _textManager->setColor(15);
        else if (cChar == 'a') {
            char* bufferPos;
            char buffer[30];
            for (bufferPos = buffer; *_textPos != '/'; *bufferPos++ = *_textPos++);
            *bufferPos = '\0';
            _textPos++;
            _idleAnim = _speakerSpr.nameToAnimId(buffer);
            for (bufferPos = buffer; *_textPos != '/'; *bufferPos++ = *_textPos++);
            *bufferPos = '\0';
            _textPos++;
            _talkAnim = _speakerSpr.nameToAnimId(buffer);
        }
        else if (cChar == 'b') {
            char* bufferPos;
            char buffer[30];
            for (bufferPos = buffer; *_textPos != '/'; *bufferPos++ = *_textPos++);
            *bufferPos = '\0';
            _textPos++;
            if (_target != nullptr)
                _idleAnim2 = _target->nameToAnimId(buffer);
            for (bufferPos = buffer; *_textPos != '/'; *bufferPos++ = *_textPos++);
            *bufferPos = '\0';
            _textPos++;
            if (_target != nullptr)
                _talkAnim2 = _target->nameToAnimId(buffer);
        }
        return;
    } else if (cChar == '\n') {
        if (_centered && !draw)
            drawTextCentered();
        _y += _lineSpacing;
        _x = _startingX;
        _lineStartColor = _textManager->getColor();
        _lineStart = _textPos;
        return;
    }
    if (draw)
        _typeSnd.play();

    if (clear && _centered)
        clearText();

    if (_centered) {
        if (draw)
            drawTextCentered();
    }
    else
        _textManager->drawGlyph(_fnt, cChar, _x, _y);
}

void Dialogue::clearText() {
    _textPos--;
    u16 width = getLineWidth();
    u8 color = _textManager->getColor();
    _textManager->setColor(0); // clear color
    _x = 128 - width / 2;
    for (char* pLine = _lineStart; pLine < _textPos; pLine++) {
        if (*pLine == '@') {
            pLine++;
            char command = *pLine;
            if (command == 'a' or command == 'b') {
                for(;*pLine != '/';pLine++);
                pLine++;
                for(;*pLine != '/';pLine++);
            }
            continue;
        }
        _textManager->drawGlyph(_fnt, *pLine, _x, _y);
        _x += 1;
    }
    _textManager->setColor(color);
    _textPos++;
}

void Dialogue::drawTextCentered() {
    u16 width = getLineWidth();
    _x = 128 - width / 2;
    _textManager->setColor(_lineStartColor); // clear color
    for (char* pLine = _lineStart; pLine < _textPos; pLine++) {
        if (*pLine == '@') {
            pLine++;
            char command = *pLine;
            if (command == '0')
                _textManager->setColor(8);
            else if (command == '1')
                _textManager->setColor(9);
            else if (command == '2')
                _textManager->setColor(10);
            else if (command == '3')
                _textManager->setColor(11);
            else if (command == '4')
                _textManager->setColor(12);
            else if (command == '5')
                _textManager->setColor(13);
            else if (command == '6')
                _textManager->setColor(14);
            else if (command == 'w')
                _textManager->setColor(15);
            else if (command == 'a' or command == 'b') {
                for(;*pLine != '/';pLine++);
                pLine++;
                for(;*pLine != '/';pLine++);
            }
            continue;
        }
        _textManager->drawGlyph(_fnt, *pLine, _x, _y);
        _x += 1;
    }
}

void Dialogue::free_() {
    _speakerSpr.setShown(false);
}
