//
// Created by cervi on 11/11/2022.
//

#include "FreeZoneManager.hpp"

namespace Engine {
    int FreeZoneManager::reserve(u16 length, u16 &start, u16 alignment) {
        char buffer[100];

        int freeZoneIdx = 0;
        u16 length_ = 0, alignOffset = 0;
        for (; freeZoneIdx < _zoneCount; freeZoneIdx++) {
            start = _zones[freeZoneIdx * 2];
            length_ = _zones[freeZoneIdx * 2 + 1];
            alignOffset = (alignment - (start % alignment)) % alignment;
            if (length_ >= alignOffset + length)
                break;
        }

        if (freeZoneIdx >= _zoneCount) {
            sprintf(buffer, "FZM %s error reserve %d length %d (needed %d) alignment %d",
                    _name, start, length_, length, alignment);
            nocashMessage(buffer);
#ifdef DEBUG_ZONES_DUMP
            dump();
#endif
            return 1;
        }

        if (alignOffset == 0) {
            if (length_ == length) {
                // Remove free zone
                auto newFreeZone = new u16[--_zoneCount * 2];

                memcpy(newFreeZone, _zones, freeZoneIdx * 2 * sizeof(u16));

                memcpy(&newFreeZone[freeZoneIdx * 2], &_zones[freeZoneIdx * 2 + 2],
                       (_zoneCount - freeZoneIdx) * 2 * sizeof(u16));

                delete[] _zones;
                _zones = newFreeZone;
            } else {
                _zones[freeZoneIdx * 2] += length;
                _zones[freeZoneIdx * 2 + 1] -= length;
            }
        } else {
            _zones[freeZoneIdx * 2 + 1] = alignOffset;
            start += alignOffset;
            length_ -= alignOffset;
            // if the length_ == length then we don't have to do anything
            // as just trimming the length of the zone is enough
            // otherwise we must create another free zone after this one
            if (length != length_) {
                // Create free zone
                auto newFreeZone = new u16[++_zoneCount * 2];

                // copy all tiles including the freeZoneIdx
                memcpy(newFreeZone, _zones, (freeZoneIdx + 1) * 2 * sizeof(u16));

                // copy all tiles after the freeZoneIdx leaving a gap
                memcpy(&newFreeZone[(freeZoneIdx + 2) * 2], &_zones[(freeZoneIdx + 1) * 2],
                       (_zoneCount - freeZoneIdx - 2) * 2 * sizeof(u16));

                newFreeZone[(freeZoneIdx + 1) * 2] = start + length;
                newFreeZone[(freeZoneIdx + 1) * 2 + 1] = length_ - length;

                delete[] _zones;
                _zones = newFreeZone;
            }
        }

#ifdef DEBUG_ZONES
        sprintf(buffer, "FZM %s reserve %d (align %d) -> start %d", _name, length, alignment, start);
        nocashMessage(buffer);
#endif

#ifdef DEBUG_ZONES_DUMP
        dump();
#endif

        return 0;
    }

#ifdef DEBUG_ZONES_DUMP
    void FreeZoneManager::dump() {
        char buffer[100];
        sprintf(buffer, "FZM %s DUMP", _name);
        nocashMessage(buffer);
        for (int i = 0; i < _zoneCount; i++) {
            sprintf(buffer, "ZONE %d (%d)", _zones[i * 2], _zones[i * 2 + 1]);
            nocashMessage(buffer);
        }
        nocashMessage("----------------------------------");
    }
#endif

    void FreeZoneManager::free(u16 length, u16 start) {
#ifdef DEBUG_ZONES
        char buffer[100];
        sprintf(buffer, "FZM %s free %d (%d)", _name, start, length);
        nocashMessage(buffer);
#endif

        int freeAfterIdx = 0;
        for (; freeAfterIdx < _zoneCount; freeAfterIdx++) {
            if (_zones[freeAfterIdx * 2] > start)
                break;
        }

        bool mergePrev = false, mergePost = false;

        // merge prev if start2 + length2 = start
        if (freeAfterIdx > 0)
            mergePrev = (_zones[freeAfterIdx * 2 - 2] + _zones[freeAfterIdx * 2 - 1]) == start;

        // merge post if start + length = start2
        if (freeAfterIdx <= _zoneCount - 1)
            mergePost = (start + length) == _zones[freeAfterIdx * 2];

        if (mergePost && mergePrev)
        {
            auto* newFreeZones = new u16[--_zoneCount * 2];

            // copy all zones before the free one
            memcpy(newFreeZones, _zones, freeAfterIdx * 2 * sizeof(u16));

            // add the length of the one we are freeing and the post
            newFreeZones[freeAfterIdx * 2 - 1] += length + _zones[freeAfterIdx * 2 + 1];

            // copy all remaining zones
            memcpy(&newFreeZones[freeAfterIdx * 2], &_zones[(freeAfterIdx + 1) * 2],
                   (_zoneCount - freeAfterIdx) * 2 * sizeof(u16));

            delete[] _zones;
            _zones = newFreeZones;
        }
        else if (mergePrev)
        {
            // add length to the previous one
            _zones[freeAfterIdx * 2 - 1] += length;
        }
        else if (mergePost)
        {
            // add length to the start and length of the post
            _zones[freeAfterIdx * 2] -= length;
            _zones[freeAfterIdx * 2 + 1] += length;
        }
        else
        {
            auto* newFreeZones = new u16[2 * ++_zoneCount];

            memcpy(newFreeZones, _zones, freeAfterIdx * 2 * sizeof(u16));

            newFreeZones[freeAfterIdx * 2] = start;
            newFreeZones[freeAfterIdx * 2 + 1] = length;

            memcpy(&newFreeZones[(freeAfterIdx + 1) * 2], &_zones[freeAfterIdx * 2],
                   (_zoneCount - (freeAfterIdx + 1)) * 2 * sizeof(u16));

            delete[] _zones;
            _zones = newFreeZones;
        }

#ifdef DEBUG_ZONES_DUMP
        dump();
#endif
    }
}
