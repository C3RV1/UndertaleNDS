//
// Created by cervi on 04/03/2026
//

#ifndef UNDERTALE_COLOREFFECTS_HPP
#define UNDERTALE_COLOREFFECTS_HPP

#include <nds.h>

namespace Engine {
void setForegroundOpacity(u8 opacity);
void setDarkenBackground(u8 intensity);
void disableColorEffects();
void setForegroundOpacitySub(u8 opacity);
void setDarkenBackgroundSub(u8 intensity);
void disableColorEffectsSub();
void setScreenBrightness(s8 brightness);
void setScreenBrightnessSub(s8 brightness);

extern u32 bldCnt, bldAlpha, bldY;
extern u32 bldCntSub, bldAlphaSub, bldYSub;
extern u32 masterBright, masterBrightSub;
} // namespace Engine

#endif
