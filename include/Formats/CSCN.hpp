//
// Created by cervi on 30/08/2022.
//

#ifndef LAYTON_CSCN_HPP
#define LAYTON_CSCN_HPP

#include <stdint.h>

struct CSCN {
    char header[4] = {'C', 'S', 'C', 'N'};
    uint32_t version = 1;
    uint32_t fileSize = 0;
    uint8_t* commands = nullptr;
};

#endif //LAYTON_CSCN_HPP
