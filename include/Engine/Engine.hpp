#ifndef UNDERTALE_ENGINE_HPP
#define UNDERTALE_ENGINE_HPP

#include <nds.h>
#include <fat.h>
#include <ctime>
#include <string>
#include "Engine/Background.hpp"

namespace Engine {
    int init();
    void tick();

    inline void log_(std::string message) {
        nocashMessage(message.c_str());
    }
    [[noreturn]] void throw_(std::string message);
}

#endif
