//
// Created by cervi on 11/11/2022.
//

#include "FreeZoneManager.hpp"

namespace Engine {
    int FreeZoneManager::reserve(u16 length, u16 &start, u16 alignment) {
        char buffer[100];

        auto freeZoneIdx = _zones.begin();
        u16 length_ = 0, alignOffset = 0;
        for (; freeZoneIdx != _zones.end(); freeZoneIdx++) {
            start = freeZoneIdx->first;
            length_ = freeZoneIdx->second;
            alignOffset = (alignment - (start % alignment)) % alignment;
            if (length_ >= alignOffset + length)
                break;
        }

        if (freeZoneIdx >= _zones.end()) {
            sprintf(buffer, "FZM %s error reserve %d length %d (needed %d) alignment %d",
                    _name.c_str(), start, length_, length, alignment);
            nocashMessage(buffer);
#ifdef DEBUG_ZONES_DUMP
            dump();
#endif
            return 1;
        }

        if (alignOffset == 0) {
            if (length_ == length) {
                // Remove free zone
                _zones.erase(freeZoneIdx);
            } else {
                freeZoneIdx->first += length;
                freeZoneIdx->second -= length;
            }
        } else {
            freeZoneIdx->second = alignOffset;
            start += alignOffset;
            length_ -= alignOffset;
            // if the length_ == length then we don't have to do anything
            // as just trimming the length of the zone is enough
            // otherwise we must create another free zone after this one
            if (length != length_) {
                // Create free zone
                _zones.emplace(freeZoneIdx + 1, start + length, length_ - length);
            }
        }

#ifdef DEBUG_ZONES
        sprintf(buffer, "FZM %s reserve %d (align %d) -> start %d", _name.c_str(), length, alignment, start);
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
        sprintf(buffer, "FZM %s DUMP %zu", _name.c_str(), _zones.size());
        nocashMessage(buffer);
        for (auto const& zone : _zones) {
            sprintf(buffer, "ZONE %d (%d)", zone.first, zone.second);
            nocashMessage(buffer);
        }
        nocashMessage("----------------------------------");
    }
#endif

    void FreeZoneManager::free(u16 length, u16 start) {
#ifdef DEBUG_ZONES
        char buffer[100];
        sprintf(buffer, "FZM %s free %d (%d)", _name.c_str(), start, length);
        nocashMessage(buffer);
#endif

        auto freeAfterIdx = _zones.begin();
        for (; freeAfterIdx != _zones.end(); freeAfterIdx++) {
            if (freeAfterIdx->first > start)
                break;
        }

        bool mergePrev = false, mergePost = false;

        // merge prev if start2 + length2 = start
        if (freeAfterIdx > _zones.begin())
            mergePrev = ((freeAfterIdx - 1)->first + (freeAfterIdx-1)->second) == start;

        // merge post if start + length = start2
        if (freeAfterIdx <= _zones.end())
            mergePost = (start + length) == freeAfterIdx->first;

        if (mergePost && mergePrev)
        {
            // add the length of the one we are freeing and the post
            (freeAfterIdx - 1)->second += length + freeAfterIdx->second;
            _zones.erase(freeAfterIdx);
        }
        else if (mergePrev)
        {
            // add length to the previous one
           (freeAfterIdx - 1)->second += length;
        }
        else if (mergePost)
        {
            // add length to the start and length of the post
            freeAfterIdx->first -= length;
            freeAfterIdx->second += length;
        }
        else
        {
            _zones.emplace(freeAfterIdx, start, length);
        }

#ifdef DEBUG_ZONES_DUMP
        dump();
#endif
    }
}
