//
// Created by cervi on 31/08/2022.
//

#ifndef UNDERTALE_DIALOGUE_HPP
#define UNDERTALE_DIALOGUE_HPP

#include <nds.h>
#include "Engine/Audio.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Font.hpp"
#include <cstdio>

class Dialogue {
public:
    Dialogue(bool centered, u16 textId, const char* speaker, s32 speakerX, s32 speakerY,
             const char* idleAnimTxt, const char* talkAnimTxt, Engine::Sprite* target,
             const char* idleAnim2Txt, const char* talkAnim2Txt, const char* typeSndPath,
             const char* fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager);
    Dialogue(bool centered_, int x_, int y_, const char* text_, const char* typeSndPath,
             const char* fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager);
    ~Dialogue() {free_();}
    bool update();
private:
    void free_();
    void setTalk();
    void setNoTalk();
    void progressText(bool clear, bool draw);
    void clearText();
    void drawTextCentered();  // Draws text centered
    u16 getLineWidth(char* pos);
    bool _paused = false;
    int _startingX, _startingY;
    int _x, _y;

    bool _centered;
    char* _lineStart = nullptr;
    char* _lastPrintedPos = nullptr;
    u16 _textLen = 0;
    u8 _lineStartColor = 15;
    const u16 _lineSpacing = 20;

    u16 _cTimer;
    u16 _letterFrames = 20;

    Engine::Texture _speakerTex;
    Engine::Sprite _speakerSpr;
    Engine::Sprite* _target;
    Engine::TextBGManager* _textManager;
    int _idleAnim = -1, _talkAnim = -1, _idleAnim2 = -1, _talkAnim2 = -1;
    char* _text = nullptr;
    char* _textPos = nullptr;
    char* _textEnd = nullptr;

    Audio::WAV _typeSnd;

    Engine::Font _fnt;

    Engine::Texture _heartTexture;
    Engine::Sprite _heartSprite;
    bool _choosingOption = false;
    int _optionCount = 0;
    int _lineOptionStart = 0;
    int _optionPositions[4][2];
    void updateChoosingOption();
    int _currentOption = 0;
};

#endif //UNDERTALE_DIALOGUE_HPP
