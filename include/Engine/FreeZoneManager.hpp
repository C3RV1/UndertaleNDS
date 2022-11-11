//
// Created by cervi on 11/11/2022.
//

#ifndef UNDERTALE_FREEZONEMANAGER_HPP
#define UNDERTALE_FREEZONEMANAGER_HPP

#define ARM9
#include <nds.h>
#include <stdio.h>
#include "DEBUG_FLAGS.hpp"

namespace Engine {
    class FreeZoneManager {
    public:
        FreeZoneManager(int start, int length, const char* name) {
            _zoneCount = 1;
            _zones = new u16[2];
            _zones[0] = start;
            _zones[1] = length;
            _name = name;
        };

        ~FreeZoneManager() {
            delete[] _zones;
            _zones = nullptr;
        }

        int reserve(u16 length, u16 &start, u16 alignment);

        void free(u16 length, u16 start);

#ifdef DEBUG_ZONES
        void dump();
#endif

    private:
        u16 _zoneCount;
        u16 *_zones;
        const char* _name;
    };
}

#endif //UNDERTALE_FREEZONEMANAGER_HPP
