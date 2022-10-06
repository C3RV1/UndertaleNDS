//
// Created by cervi on 30/08/2022.
//

#ifndef UNDERTALE_CSCN_HPP
#define UNDERTALE_CSCN_HPP

#define ARM9
#include <nds.h>

struct CSCN {
    char header[4] = {'C', 'S', 'C', 'N'};
    u32 version = 6;
    u32 fileSize = 0;
    u8* commands = nullptr;
};

#endif //UNDERTALE_CSCN_HPP
