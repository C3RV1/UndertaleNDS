//
// Created by cervi on 31/08/2022.
//

#ifndef UNDERTALE_DIALOGUE_HPP
#define UNDERTALE_DIALOGUE_HPP

class Dialogue;

#define ARM9
#include <nds.h>
#include "OAMManager.hpp"
#include "Cutscene.hpp"
#include "Font.hpp"
#include <stdio.h>

class Dialogue {
public:
    Dialogue(bool isRoom_, uint16_t textId, char* speaker, int32_t speakerX, int32_t speakerY,
             char* idleAnimTxt, char* talkAnimTxt, Engine::Sprite* target_,
             char* idleAnim2Txt, char* talkAnim2Txt, char* typeSndPath,
             char* fontTxt, uint16_t framesPerLetter);
    bool update();
    void free_();
private:
    void setTalk();
    void setNoTalk();
    void progressText(bool clear, bool draw);
    void progressTextRoom(bool clear, bool draw);  // Draws text centered
    void progressTextBattle(bool clear, bool draw);  // Draws text left-aligned
    uint16_t getLineWidth(int linePos_);
    void getLine();
    bool paused = false;
    int startingX, startingY;
    int x, y;

    bool isRoom;
    uint16_t linePos = 0;
    uint16_t lineLen = 0;
    uint16_t lineWidth = 0;
    uint16_t textLen = 0;
    const uint16_t lineSpacing = 20;

    uint16_t currentTimer;
    uint16_t letterFrames = 20;

    Engine::Texture speakerSpr;
    Engine::Sprite speakerManager;
    Engine::Sprite* target;
    int idleAnim = -1, talkAnim = -1, idleAnim2 = -1, talkAnim2 = -1;
    FILE* textStream;
    char line[100] = {};
    uint8_t currentColor = 15;

    BGM::WAV typeSnd;

    Engine::Font font;
};

extern Dialogue* currentDialogue;

#endif //UNDERTALE_DIALOGUE_HPP
