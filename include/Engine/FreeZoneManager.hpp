//
// Created by cervi on 11/11/2022.
//

#ifndef UNDERTALE_FREEZONEMANAGER_HPP
#define UNDERTALE_FREEZONEMANAGER_HPP

#include <nds.h>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "DEBUG_FLAGS.hpp"

namespace Engine {
    class FreeZoneManager {
    public:
        FreeZoneManager(int start, int length, std::string name) {
            _zones.emplace_back(start, length);
            _name = std::move(name);
        };

        int reserve(u16 length, u16 &start, u16 alignment);

        void free(u16 length, u16 start);

#ifdef DEBUG_ZONES_DUMP
        void dump();
#endif

    private:
        std::vector<std::pair<u16, u16>> _zones;
        std::string _name;
    };
}

#endif //UNDERTALE_FREEZONEMANAGER_HPP
