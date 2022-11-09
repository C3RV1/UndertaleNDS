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
        _speakerSpr(Engine::AllocatedOAM),
        _centered(centered), _textManager(&txtManager), _target(target) {
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
        getLine();

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
        if (((keysDown() & (/*KEY_TOUCH |*/ KEY_B)) || _letterFrames == 0) && (_textPos != _textLen)) {
            progressText(true, false);
            while (!_paused && !(_textPos >= _textLen && (_linePos >= _lineLen || !_centered)))
                progressText(false, false);
            if (_centered && _linePos > 0) {
                _linePos--;
                progressText(false, true);
            }
        }
        if (_textPos == _textLen && (_linePos >= _lineLen || !_centered)) {
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

u16 Dialogue::getLineWidth(int linePos_) {
    u16 lineWidth_ = 0;
    for (char* pLine = _line; pLine < _line + linePos_; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        lineWidth_ += _fnt.getGlyphWidth(*pLine);
        lineWidth_++;
    }
    return lineWidth_ - 1;
}

void Dialogue::getLine() {
    if (_text == nullptr)
        return;
    for (_lineLen = 0; *(_text + _textPos + _lineLen) != '\n'; _lineLen++);
    memcpy(_line, _text + _textPos, _lineLen + 1);
    _line[_lineLen] = '\0';
    _textPos += _lineLen + 1;
}

void Dialogue::progressText(bool clear, bool draw) {
    if (_centered)
        progressTextCentered(clear, draw);
    else
        progressTextLeft(clear, draw);
}

void Dialogue::progressTextCentered(bool clear, bool draw) {
    if (_cTimer > 0 && draw) {
        _cTimer--;
        return;
    }
    _cTimer = _letterFrames;
    if (_linePos == _lineLen) {
        if (_textPos >= _textLen) {
            return;
        }
        _y += _lineSpacing;
        _linePos = 0;
        getLine();
    }
    if (*(_line + _linePos) == '@') {
        _linePos++;
        char command = *(_line + _linePos);
        _linePos++;
        if (command == 'p') {
            _paused = true;
        }
        else if (command == 'c') {
            _textManager->clear();
            _y = _startingY;
        }
        else if (command == 'a') {
            char buffer[30];
            char* pBuffer = buffer;
            for (char* pLine = (_line + _linePos); *pLine != '/';
                 pLine++, pBuffer++, _linePos++) {
                *pBuffer = *pLine;
            }
            _linePos++;
            *pBuffer = '\0';
            _idleAnim = _speakerSpr.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (_line + _linePos); *pLine != '/';
                 pLine++, pBuffer++, _linePos++) {
                *pBuffer = *pLine;
            }
            _linePos++;
            *pBuffer = '\0';
            _talkAnim = _speakerSpr.nameToAnimId(buffer);
        }
        else if (command == 'b') {
            char buffer[30];
            char* pBuffer = buffer;
            for (char* pLine = (_line + _linePos); *pLine != '/';
                 pLine++, pBuffer++, _linePos++) {
                *pBuffer = *pLine;
            }
            _linePos++;
            *pBuffer = '\0';
            _idleAnim2 = _speakerSpr.nameToAnimId(buffer);
            pBuffer = buffer;
            for (char* pLine = (_line + _linePos); *pLine != '/';
                 pLine++, pBuffer++, _linePos++) {
                *pBuffer = *pLine;
            }
            _linePos++;
            *pBuffer = '\0';
            _talkAnim2 = _speakerSpr.nameToAnimId(buffer);
        }
        _cTimer = 0;
        return;
    }
    _linePos++;
    if (draw && _linePos < _lineLen)
        _typeSnd.play();

    // clear current chars
    u16 width = getLineWidth(_linePos - 1);
    _startingX = 128 - width / 2;
    _textManager->setColor(0); // clear color
    for (char* pLine = _line; pLine < _line + _linePos - 1; pLine++) {
        if (*pLine == '@') {
            pLine += 1;
            continue;
        }
        if (clear || _linePos > _lineLen)
            _textManager->drawGlyph(_fnt, *pLine, _startingX, _y);
        _startingX += 1;
    }

    width = getLineWidth(_linePos);
    _startingX = 128 - width / 2;
    _textManager->setColor(_cColor); // clear color
    int lineEndColor = _cColor;
    for (char* pLine = _line; pLine < _line + _linePos; pLine++) {
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
            lineEndColor = _textManager->getColor();
            continue;
        }
        if (draw || _linePos == _lineLen)
            _textManager->drawGlyph(_fnt, *pLine, _startingX, _y);
        _startingX += 1;
    }

    if (_linePos == _lineLen)
        _cColor = lineEndColor;
}

void Dialogue::progressTextLeft(bool, bool draw) {
    if (_cTimer > 0 && draw) {
        _cTimer--;
        return;
    }
    _cTimer = _letterFrames;
    if (_textPos >= _textLen) {
        return;
    }

    char cChar;
    if (_text != nullptr)
        cChar = _text[_textPos++];
    else
        return;

    if (cChar == '@') {
        cChar = _text[_textPos++];
        if (cChar == 'p') {
            _paused = true;
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
            int len;
            for (len = 0; *(_text + _textPos + len) != '/'; len++);
            char buffer[30];
            memcpy(buffer, _text + _textPos, len + 1);
            buffer[len] = '\0';
            _textPos += len + 1;
            _idleAnim = _speakerSpr.nameToAnimId(buffer);
            for (len = 0; *(_text + _textPos + len) != '/'; len++);
            memcpy(buffer, _text + _textPos, len + 1);
            buffer[len] = '\0';
            _textPos += len + 1;
            _talkAnim = _speakerSpr.nameToAnimId(buffer);
        }
        else if (cChar == 'b') {
            int len;
            for (len = 0; *(_text + _textPos + len) != '/'; len++);
            char buffer[30];
            memcpy(buffer, _text + _textPos, len + 1);
            buffer[len] = '\0';
            _textPos += len + 1;
            if (_target != nullptr)
                _idleAnim2 = _target->nameToAnimId(buffer);
            for (len = 0; *(_text + _textPos + len) != '/'; len++);
            memcpy(buffer, _text + _textPos, len + 1);
            buffer[len] = '\0';
            _textPos += len + 1;
            if (_target != nullptr)
                _talkAnim2 = _target->nameToAnimId(buffer);
        }
        return;
    } else if (cChar == '\n') {
        _y += _lineSpacing;
        _x = _startingX;
        return;
    }
    if (draw)
        _typeSnd.play();

    _textManager->drawGlyph(_fnt, cChar, _x, _y);
}

void Dialogue::free_() {
    _speakerSpr.setShown(false);
    _speakerTex.free_();
    _typeSnd.stop();
    _typeSnd.free_();
}
