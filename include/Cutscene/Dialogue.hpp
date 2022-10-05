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
#include <stdio.h>

class Dialogue {
public:
    Dialogue(bool isRoom_, u16 textId, const char* speaker, s32 speakerX, s32 speakerY,
             const char* idleAnimTxt, const char* talkAnimTxt, Engine::Sprite* target_,
             const char* idleAnim2Txt, const char* talkAnim2Txt, const char* typeSndPath,
             const char* fontTxt, u16 framesPerLetter);
    Dialogue(bool isRoom_, int x_, int y_, const char* text_, const char* typeSndPath,
             const char* fontTxt, u16 framesPerLetter, Engine::TextBGManager& txtManager);
    bool update();
    void free_();
private:
    void setTalk();
    void setNoTalk();
    void progressText(bool clear, bool draw);
    void progressTextRoom(bool clear, bool draw);  // Draws text centered
    void progressTextBattle(bool clear, bool draw);  // Draws text left-aligned
    u16 getLineWidth(int linePos_);
    void getLine();
    bool paused = false;
    int startingX, startingY;
    int x, y;

    bool isRoom;
    u16 linePos = 0;
    u16 lineLen = 0;
    u16 textPos = 0;
    u16 textLen = 0;
    const u16 lineSpacing = 20;

    u16 currentTimer;
    u16 letterFrames = 20;

    Engine::Texture speakerSpr;
    Engine::Sprite speakerManager;
    Engine::Sprite* target;
    Engine::TextBGManager* textManager;
    int idleAnim = -1, talkAnim = -1, idleAnim2 = -1, talkAnim2 = -1;
    char* text = nullptr;
    char line[100] = {};
    u8 currentColor = 15;

    Audio::WAV typeSnd;

    Engine::Font font;
};

#endif //UNDERTALE_DIALOGUE_HPP
