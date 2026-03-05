//
// Created by cervi on 04/03/2026
//

#ifndef UNDERTALE_COLOREFFECTS_HPP
#define UNDERTALE_COLOREFFECTS_HPP

#include <nds.h>

namespace Engine {
void setForegroundOpacity(s8 opacity);
void setDarkenBackground(s8 intensity);
void disableColorEffects();
void setForegroundOpacitySub(s8 opacity);
void setDarkenBackgroundSub(s8 intensity);
void disableColorEffectsSub();

extern s32 bldCnt, bldAlpha, bldY;
extern s32 bldCntSub, bldAlphaSub, bldYSub;
} // namespace Engine

#endif
