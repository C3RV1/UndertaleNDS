//
// Created by cervi on 31/08/2022.
//

#include "Cutscene/Dialogue.hpp"
#include "Engine/OAMManager.hpp"
#include "Engine/Engine.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Formats/utils.hpp"

Dialogue::Dialogue(u16 textId, Engine::Sprite* target,
                   const std::string& targetIdle, const std::string& targetTalk,
                   const std::string& typeSndPath, const std::string& fontTxt,
                   u16 framesPerLetter, Engine::TextBGManager& txtManager,
                   Engine::AllocationMode heartAlloc) :
        _target(target), _textManager(&txtManager), _heartSprite(heartAlloc) {
    _fnt.loadPath(fontTxt);
    _heartTexture.loadPath("spr_heartsmall");
    _heartSprite.loadTexture(_heartTexture);

    std::string buffer =
            "nitro:/data/dialogue/r" + std::to_string(globalCutscene->_roomId) +
            "/c" + std::to_string(globalCutscene->_cutsceneId) +
            "/d" + std::to_string(textId) + ".txt";
    FILE* textStream = fopen(buffer.c_str(), "rb");
    if (textStream == nullptr) {
        Engine::throw_("Error opening text file #r" + buffer);
    }
    u32 textLen = str_len_file(textStream, '\0');
    _text.resize(textLen);
    fread(&_text[0], textLen, 1, textStream);

    _textPos = _text.begin();
    _lastClear = _text.begin();

    _letterFrames = framesPerLetter;
    _cTimer = _letterFrames;

    if (target != nullptr) {
        _targetIdle = target->nameToAnimId(targetIdle);
        _targetTalk = target->nameToAnimId(targetTalk);
    }

    _textManager->clear();
    _textManager->reloadColors();
    _textManager->setColor(15);

    _typeSnd.loadWAV(typeSndPath);
    _typeSnd.setLoops(0);
}

Dialogue::Dialogue(const std::string& text_, const std::string& typeSndPath, const std::string& fontTxt,
                   u16 framesPerLetter, Engine::TextBGManager &txtManager, Engine::AllocationMode heartAlloc) :
        _textManager(&txtManager), _heartSprite(heartAlloc) {
    _fnt.loadPath(fontTxt);
    _heartTexture.loadPath("spr_heartsmall");
    _heartSprite.loadTexture(_heartTexture);

    _text = text_;

    _textPos = _text.begin();
    _lastClear = _text.begin();

    _letterFrames = framesPerLetter;
    _cTimer = _letterFrames;

    _textManager->clear();
    _textManager->reloadColors();
    _textManager->setColor(15);

    _typeSnd.loadWAV(typeSndPath);
    _typeSnd.setLoops(0);
}

void Dialogue::setTalk() {
    if (_target != nullptr)
        _target->setSpriteAnim(_targetTalk);
}

void Dialogue::setNoTalk() {
    if (_target != nullptr)
        _target->setSpriteAnim(_targetIdle);
}

bool Dialogue::update() {
    if (_textPos >= _text.end() && !_paused) {
        setNoTalk();
        return true;
    }
    if (!_paused) {
        setTalk();
        progressText(true, true);
        if (((keysDown() & (/*KEY_TOUCH |*/ KEY_B)) || _letterFrames == 0)) {
            progressText(true, false);
            while (!_paused && _textPos < _text.end())
                progressText(false, false);
        }
        return false;
    } else {
        setNoTalk();
        if (_choosingOption) {
            updateChoosingOption();
            return false;
        }
        if (keysDown() & (/*KEY_TOUCH |*/ KEY_A)) {
            _paused = false;
            // progressText(true, true);
            return false;
        }
    }
    return false;
}

int Dialogue::sizeInline(std::string::iterator &posToProgress) {
    char cChar = *posToProgress++;
    if (cChar == 'b') {
        for(;*posToProgress != '/';posToProgress++);
        posToProgress++;
        for(;*posToProgress != '/';posToProgress++);
    }
    else if (cChar == 'o') {
        if (*(++posToProgress) == 'p')
            return 11;
    }
    return 0;
}

void Dialogue::progressText(bool clear_, bool draw_) {
    if (_cTimer > 0 && draw_) {
        _cTimer--;
        return;
    }
    _cTimer = _letterFrames;
    char cChar = *_textPos++;

    if (cChar == '@') {
        handleInline(_textPos, true);
        _cTimer = 0;
        return;
    } else if (cChar == '\n') {
        onLineBreak();
        return;
    }
    if (draw_)
        _typeSnd.play();

    draw(draw_, clear_);
}

void Dialogue::updateChoosingOption() {
    _heartSprite.setShown(true);
    if (keysDown() & KEY_RIGHT) {
        _currentOption++;
        _currentOption %= _optionCount;
    }
    else if (keysDown() & KEY_LEFT) {
        _currentOption--;
        if (_currentOption < 0)
            _currentOption = 0;
    }
    else if (keysDown() & KEY_DOWN && _optionCount > 2) {
        _currentOption += 2;
        _currentOption %= _optionCount;
    }
    else if (keysDown() & KEY_UP && _optionCount > 2) {
        _currentOption -= 2;
        if (_currentOption < 0)
            _currentOption = 0;
    }
    else if (keysDown() & KEY_A) {
        _paused = false;
        _choosingOption = false;
        _optionCount = 0;
        globalSave.flags[FlagIds::DIALOGUE_OPTION] = _currentOption;
        _heartSprite.setShown(false);
        return;
    }
    _heartSprite._wx = (_optionPositions[_currentOption][0] - 2) << 8;
    _heartSprite._wy = (_optionPositions[_currentOption][1] + 4) << 8;
}

void Dialogue::onClear() {
    _textManager->clear();
    _lastClear = _textPos;
}

void Dialogue::handleInline(std::string::iterator& pos, bool doEffect) {
    char cChar = *pos++;
    if (cChar == 'p') {
        if (doEffect)
            onPause();
    }
    else if (cChar == 'c') {
        if (doEffect)
            onClear();
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
    else if (cChar == 'b') {
        std::string buffer;
        buffer.reserve(30);
        for (; *pos != '/'; buffer += *pos++);
        pos++;
        if (_target != nullptr and doEffect)
            _targetIdle = _target->nameToAnimId(buffer);
        buffer = "";
        for (; *pos != '/'; buffer += *pos++);
        pos++;
        if (_target != nullptr and doEffect)
            _targetTalk = _target->nameToAnimId(buffer);
    }
    else if (cChar == 'o') {
        // Options
        cChar = *pos++;
        if (cChar == 'p') {
            // present
            if (_optionCount < 4) {
                _optionPositions[_optionCount][0] = _x;
                _optionPositions[_optionCount][1] = _y;
                _optionCount++;
            }
            _x += 11;
        } else if (cChar == 'c') {
            if (doEffect)
                onOptionChoose();
        }
    }
}

void Dialogue::onPause() {
    _paused = true;
}

void Dialogue::onOptionChoose() {
    _paused = true;
    _choosingOption = true;
    _currentOption = 0;
}

void Dialogue::onLineBreak() {
    _y += _lineSpacing;
}

void Dialogue::doRedraw() {
    auto currentPos = _textPos;
    _textPos = _lastClear;  // Rewind to _lastClear
    onClear();
    progressText(true, false);
    while (_textPos < currentPos)
        progressText(false, false);
}
