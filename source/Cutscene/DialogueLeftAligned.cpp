//
// Created by cervi on 28/08/2023.
//
#include "Cutscene/Dialogue.hpp"

DialogueLeftAligned::DialogueLeftAligned(u16 textId, s32 startingX, s32 startingY, Engine::Sprite *target,
                                         const std::string& targetIdle, const std::string& targetTalk,
                                         const std::string& typeSndPath, const std::string& fontTxt,
                                         u16 framesPerLetter, Engine::TextBGManager &txtManager,
                                         Engine::AllocationMode heartAlloc) :
        Dialogue(textId, target, targetIdle, targetTalk, typeSndPath, fontTxt, framesPerLetter, txtManager,
                 heartAlloc) {
    _startingX = startingX >> 8;
    _x = _startingX;
    _startingY = startingY >> 8;
    _y = _startingY;
}

DialogueLeftAligned::DialogueLeftAligned(int startingX, int startingY, const std::string& text_, const std::string& typeSndPath,
                                         const std::string& fontTxt, u16 framesPerLetter,
                                         Engine::TextBGManager &txtManager, Engine::AllocationMode heartAlloc) :
        Dialogue(text_, typeSndPath, fontTxt, framesPerLetter, txtManager, heartAlloc){
    _startingX = startingX >> 8;
    _x = _startingX;
    _startingY = startingY >> 8;
    _y = _startingY;
}

void DialogueLeftAligned::onClear() {
    Dialogue::onClear();
    _x = _startingX;
    _y = _startingY;
}

void DialogueLeftAligned::draw(bool draw_, bool clear_) {
    _textManager->drawGlyph(_fnt, *(_textPos - 1), _x, _y);
    _x += 1;
}

void DialogueLeftAligned::onLineBreak() {
    Dialogue::onLineBreak();
    _x = _startingX;
}
