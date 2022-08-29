//
// Created by cervi on 29/08/2022.
//

#ifndef LAYTON_CUTSCENE_HPP
#define LAYTON_CUTSCENE_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "Navigation.hpp"

class Cutscene {
public:
    Cutscene(uint16_t cutsceneId, Navigation* navigation);
private:
    FILE* commandStream;
};

#endif //LAYTON_CUTSCENE_HPP
