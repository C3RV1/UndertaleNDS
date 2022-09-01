//
// Created by cervi on 31/08/2022.
//

#ifndef LAYTON_DIALOGUE_HPP
#define LAYTON_DIALOGUE_HPP

class Dialogue;

#define ARM9
#include <nds.h>
#include "OAMManager.hpp"
#include "Cutscene.hpp"
#include "Font.hpp"
#include <stdio.h>

class Dialogue {
public:
    Dialogue(uint16_t textId, char* speaker, int32_t speakerX, int32_t speakerY,
             char* idleAnimTxt, char* talkAnimTxt, Engine::SpriteManager* target_,
             char* idleAnim2Txt, char* talkAnim2Txt, char* fontTxt, uint16_t framesPerLetter);
    bool update();
    void free_();
private:
    void setTalk();
    void setNoTalk();
    void progressText(bool clear, bool draw);
    uint16_t getLineWidth(int linePos_);
    void getLine();
    bool paused = false;
    uint16_t y;
    uint16_t linePos = 0;
    uint16_t lineLen = 0;
    uint16_t lineWidth = 0;
    uint16_t textLen = 0;
    const uint16_t startingY = 192 / 2, lineSpacing = 20;

    uint16_t currentTimer;
    uint16_t letterFrames = 20;

    Engine::Sprite speakerSpr;
    Engine::SpriteManager speakerManager;
    Engine::SpriteManager* target;
    int idleAnim, talkAnim, idleAnim2, talkAnim2;
    FILE* textStream;
    char line[100] = {};
    uint8_t currentColor = 15;

    Engine::Font font;
};

extern Dialogue* currentDialogue;

#endif //LAYTON_DIALOGUE_HPP
