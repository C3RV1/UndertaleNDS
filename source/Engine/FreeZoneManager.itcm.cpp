//
// Created by cervi on 11/11/2022.
//

#include "DEBUG_FLAGS.hpp"
#include "Engine/FreeZoneManager.hpp"
#include "Engine/Engine.hpp"
#include <string>

namespace Engine {
int FreeZoneManager::reserve(u16 length, u16 &start, u16 alignment) {
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
    dump();
    throw_("FZM " + _name + " error reserve " + std::to_string(start) +
           " length " + std::to_string(length_) + " (needed " +
           std::to_string(length) + ") alignment " + std::to_string(alignment));
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

  debug_zones("FZM " + _name + " reserve " + std::to_string(length) +
                  " (align " + std::to_string(alignment) + ") -> start " +
                  std::to_string(start));

  dump();

  return 0;
}

void FreeZoneManager::dump() {
#ifdef DEBUG_ZONES_DUMP
  Engine::log_("FZM " + _name + + " DUMP " + std::to_string(_zones.size()));
  for (auto const &zone : _zones) {
    Engine::log_("ZONE " + std::to_string(zone.first) + " (" +
                 std::to_string(zone.second) + ")");
  }
  Engine::log_("----------------------------------");
#endif
}

void FreeZoneManager::free(u16 length, u16 start) {
  debug_zones("FZM " + _name + " free " + std::to_string(start) + " (" +
              std::to_string(length) + ")");

  auto freeAfterIdx = _zones.begin();
  for (; freeAfterIdx != _zones.end(); freeAfterIdx++) {
    if (freeAfterIdx->first > start)
      break;
  }

  bool mergePrev = false, mergePost = false;

  // merge prev if start2 + length2 = start
  if (freeAfterIdx > _zones.begin())
    mergePrev =
        ((freeAfterIdx - 1)->first + (freeAfterIdx - 1)->second) == start;

  // merge post if start + length = start2
  if (freeAfterIdx <= _zones.end())
    mergePost = (start + length) == freeAfterIdx->first;

  if (mergePost && mergePrev) {
    // add the length of the one we are freeing and the post
    (freeAfterIdx - 1)->second += length + freeAfterIdx->second;
    _zones.erase(freeAfterIdx);
  } else if (mergePrev) {
    // add length to the previous one
    (freeAfterIdx - 1)->second += length;
  } else if (mergePost) {
    // add length to the start and length of the post
    freeAfterIdx->first -= length;
    freeAfterIdx->second += length;
  } else {
    _zones.emplace(freeAfterIdx, start, length);
  }

  dump();
}
} // namespace Engine
