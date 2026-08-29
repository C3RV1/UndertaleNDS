//
// Created by cervi on 02/10/2022.
//

#ifndef UNDERTALE_DEBUG_FLAGS_HPP
#define UNDERTALE_DEBUG_FLAGS_HPP

#define DEBUG_CUTSCENES
// #define DEBUG_2D
// #define DEBUG_3D
// #define DEBUG_TEXTURES
// #define DEBUG_AUDIO
// #define DEBUG_ZONES
// #define DEBUG_ZONES_DUMP
#define DEBUG_SAVE
#define DEBUG_FONTS
// #define DEBUG_CONDITIONAL_FILE

#include "Engine/Engine.hpp"

inline void debug_2d(std::string msg) {
#ifdef DEBUG_2D
  Engine::log_(msg);
#endif
}

inline void debug_3d(std::string msg) {
#ifdef DEBUG_3D
  Engine::log_(msg);
#endif
}

inline void debug_textures(std::string msg) {
#ifdef DEBUG_TEXTURES
  Engine::log_(msg);
#endif
}

inline void debug_audio(std::string msg) {
#ifdef DEBUG_AUDIO
  Engine::log_(msg);
#endif
}

inline void debug_zones(std::string msg) {
#ifdef DEBUG_ZONES
  Engine::log_(msg);
#endif
}

inline void debug_save(std::string msg) {
#ifdef DEBUG_SAVE
  Engine::log_(msg);
#endif
}

inline void debug_fonts(std::string msg) {
#ifdef DEBUG_FONTS
  Engine::log_(msg);
#endif
}

inline void debug_cutscene(std::string msg) {
#ifdef DEBUG_CUTSCENES
  Engine::log_(msg);
#endif
}

inline void debug_conditional_file(std::string msg) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_(msg);
#endif
}

#endif // UNDERTALE_DEBUG_FLAGS_HPP
