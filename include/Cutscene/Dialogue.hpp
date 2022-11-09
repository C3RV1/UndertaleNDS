//
// Created by cervi on 31/08/2022.
//

#ifndef UNDERTALE_DIALOGUE_HPP
#define UNDERTALE_DIALOGUE_HPP

#define ARM9
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
    bool update();
    void free_();
private:
    void setTalk();
    void setNoTalk();
    void progressText(bool clear, bool draw);
    void progressTextCentered(bool clear, bool draw);  // Draws text centered
    void progressTextLeft(bool clear, bool draw);  // Draws text left-aligned
    u16 getLineWidth(int linePos_);
    void getLine();
    bool _paused = false;
    int _startingX, _startingY;
    int _x, _y;

    bool _centered;
    u16 _linePos = 0;
    u16 _lineLen = 0;
    u16 _textPos = 0;
    u16 _textLen = 0;
    const u16 _lineSpacing = 20;

    u16 _cTimer;
    u16 _letterFrames = 20;

    Engine::Texture _speakerTex;
    Engine::Sprite _speakerSpr;
    Engine::Sprite* _target;
    Engine::TextBGManager* _textManager;
    int _idleAnim = -1, _talkAnim = -1, _idleAnim2 = -1, _talkAnim2 = -1;
    char* _text = nullptr;
    char _line[100] = {};
    u8 _cColor = 15;

    Audio::WAV _typeSnd;

    Engine::Font _fnt;
};

#endif //UNDERTALE_DIALOGUE_HPP
