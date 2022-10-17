//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_CUTSCENE_HPP
#define UNDERTALE_CUTSCENE_HPP

#define ARM9
#include <nds.h>
#include <cstdio>
#include "Waiting.hpp"
#include "Dialogue.hpp"
#include "SaveMenu.hpp"

class Cutscene {
public:
    explicit Cutscene(u16 cutsceneId_, u16 roomId_);
    static bool checkHeader(FILE *f);
    void update();
    bool runCommands(CutsceneLocation callingLocation);
    bool runCommand(CutsceneLocation callingLocation);
    u16 cutsceneId;
    u16 roomId;
    ~Cutscene();
    Dialogue* currentDialogue = nullptr;
    SaveMenu* currentSaveMenu = nullptr;
private:
    Waiting waiting;
    bool flag = false;
    FILE* commandStream = nullptr;
    long commandStreamLen = 0;
};

extern Cutscene* globalCutscene;

#endif //UNDERTALE_CUTSCENE_HPP
