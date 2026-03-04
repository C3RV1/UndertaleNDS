//
// Created by cervi on 04/03/2026
//

#ifndef UNDERTALE_COLOREFFECTS_HPP
#define UNDERTALE_COLOREFFECTS_HPP

#include <nds.h>

namespace Engine {
void setForegroundOpacity(s8 opacity);
void setDarkenBackground(s8 intensity);

extern s32 bldCntSub, bldAlphaSub, bldYSub;
} // namespace Engine

#endif
