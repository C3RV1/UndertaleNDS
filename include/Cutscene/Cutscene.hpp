//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_CUTSCENE_HPP
#define UNDERTALE_CUTSCENE_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Waiting.hpp"

class Cutscene {
public:
    explicit Cutscene(u16 cutsceneId_);
    static bool checkHeader(FILE *f);
    bool runCommands(CutsceneLocation callingLocation);
    bool runCommand(CutsceneLocation callingLocation);
    u16 cutsceneId;
    u16 roomId;
    ~Cutscene();
private:
    Waiting waiting;
    bool flag = false;
    FILE* commandStream = nullptr;
    long commandStreamLen = 0;
};

extern Cutscene* globalCutscene;

#endif //UNDERTALE_CUTSCENE_HPP
