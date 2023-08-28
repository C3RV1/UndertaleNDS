//
// Created by cervi on 28/08/2023.
//
#include "Cutscene/Dialogue.hpp"

DialogueCentered::DialogueCentered(u16 textId, const std::string& speaker, s32 speakerX, s32 speakerY,
                                   const std::string& speakerIdle, const std::string& speakerTalk,
                                   Engine::Sprite *target, const std::string& targetIdle,
                                   const std::string& targetTalk, const std::string& typeSndPath,
                                   const std::string& fontTxt, u16 framesPerLetter,
                                   Engine::TextBGManager &txtManager, Engine::AllocationMode heartAlloc) :
        Dialogue(textId, target, targetIdle, targetTalk, typeSndPath,
                 fontTxt, framesPerLetter, txtManager, heartAlloc){

    if (!speaker.empty())
        _speakerTex.loadPath(speaker);

    if (!speaker.empty() != 0)
        _startingY = 192 / 2;
    else
        _startingY = 192 / 4;
    _y = _startingY;

    _speakerSpr.loadTexture(_speakerTex);
    _speakerSpr._wx = speakerX;
    _speakerSpr._wy = speakerY;
    _speakerSpr.setShown(true);
    _speakerIdle = _speakerSpr.nameToAnimId(speakerIdle);
    _speakerTalk = _speakerSpr.nameToAnimId(speakerTalk);

    _lineStart = _text.begin();

}

// TODO: Maybe this constructor could also take a speaker?
//       Would then Dialogue need to take a target?
DialogueCentered::DialogueCentered(const std::string& text_, const std::string& typeSndPath,
                                   const std::string& fontTxt, u16 framesPerLetter, Engine::TextBGManager &txtManager,
                                   Engine::AllocationMode heartAlloc) :
        Dialogue(text_, typeSndPath, fontTxt, framesPerLetter, txtManager, heartAlloc){
    _startingY = 192 / 4;
    _y = _startingY;

    _lineStart = _text.begin();
}

void DialogueCentered::setTalk() {
    Dialogue::setTalk();
    _speakerSpr.setSpriteAnim(_speakerTalk);
}

void DialogueCentered::setNoTalk() {
    Dialogue::setNoTalk();
    _speakerSpr.setSpriteAnim(_speakerIdle);
}

void DialogueCentered::handleInline(std::string::iterator& pos, bool doEffect) {
    char cChar = *pos++;
    if (cChar == 'a') {
        std::string buffer;
        buffer.reserve(30);
        for (; *pos != '/'; buffer += *pos++);
        pos++;
        if (doEffect)
            _speakerIdle = _speakerSpr.nameToAnimId(buffer);
        buffer = "";
        for (; *pos != '/'; buffer += *pos++);
        pos++;
        if (doEffect)
            _speakerTalk = _speakerSpr.nameToAnimId(buffer);
    }
    else
        Dialogue::handleInline(--pos, doEffect);
}

DialogueCentered::~DialogueCentered() {
    _speakerSpr.setShown(false);
}

void DialogueCentered::onLineBreak() {
    draw(true, true);
    Dialogue::onLineBreak();
    _lineStart = _textPos;
    _lineOptionStart = _optionCount;
    _lineStartColor = _textManager->getColor();
}

void DialogueCentered::draw(bool draw_, bool _clear) {
    if (_clear)
        clearCentered();
    if (!draw_)
        return;
    u16 width = getLineWidth(_textPos);
    _x = 128 - width / 2;
    _textManager->setColor(_lineStartColor); // clear color
    _optionCount = _lineOptionStart;
    for (auto pLine = _lineStart; pLine < _textPos;) {
        if (*pLine == '@') {
            handleInline(++pLine, false);
            continue;
        }
        _textManager->drawGlyph(_fnt, *pLine++, _x, _y);
        _x += 1;
    }
    _lastPrintedPos = _textPos;
}

void DialogueCentered::clearCentered() {
    if (_lastPrintedPos == _text.end())
        return;
    u16 width = getLineWidth(_lastPrintedPos);
    u8 color = _textManager->getColor();
    _textManager->setColor(0); // clear color
    _x = 128 - width / 2;
    for (auto pLine = _lineStart; pLine < _lastPrintedPos;) {
        if (*pLine == '@') {
            _x += sizeInline(++pLine);
            continue;
        }
        _textManager->drawGlyph(_fnt, *pLine++, _x, _y);
        _x += 1;
    }
    _textManager->setColor(color);
}

int DialogueCentered::sizeInline(std::string::iterator &posToProgress) {
    char cChar = *posToProgress++;
    if (cChar == 'a') {
        for(;*posToProgress != '/';posToProgress++);
        posToProgress++;
        for(;*posToProgress != '/';posToProgress++);
        return 0;
    }
    return Dialogue::sizeInline(--posToProgress);
}

void DialogueCentered::onClear() {
    Dialogue::onClear();
    _y = _startingY;
}

u16 DialogueCentered::getLineWidth(std::string::iterator pos) {
    u16 lineWidth_ = 0;
    for (auto pLine = _lineStart; pLine < pos;) {
        if (*pLine == '@') {
            lineWidth_ += sizeInline(++pLine);
            continue;
        }
        lineWidth_ += _fnt.getGlyphWidth(*pLine++);
        lineWidth_++;
    }
    return lineWidth_ - 1;
}

void DialogueCentered::onPause() {
    Dialogue::onPause();
    draw(true, false);
}

void DialogueCentered::onOptionChoose() {
    Dialogue::onOptionChoose();
    draw(true, false);
}
