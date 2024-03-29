//
// Created by cervi on 30/08/2022.
//

#ifndef UNDERTALE_CSCN_HPP
#define UNDERTALE_CSCN_HPP

#include <nds.h>

struct CSCN {
    char header[4] = {'C', 'S', 'C', 'N'};
    static constexpr u32 version = 11;
    u32 fileSize = 0;
    u8* commands = nullptr;
};

#endif //UNDERTALE_CSCN_HPP
