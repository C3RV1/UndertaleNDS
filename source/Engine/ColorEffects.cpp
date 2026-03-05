#include "Engine/ColorEffects.hpp"

namespace Engine {
s32 bldCnt = 0, bldAlpha = 0, bldY = 0;
s32 bldCntSub = 0, bldAlphaSub = 0, bldYSub = 0;

void setForegroundOpacity(s8 opacity) {
  // ALPHA | BG3
  bldCnt = (1 << 6) | (1 << 11);

  if (opacity > 16)
    opacity = 16;
  bldAlpha = (opacity & 0xF) | (((16 - opacity) & 0xF) << 8);
}

void setDarkenBackground(s8 intensity) {
  // BG3 | BRIGHTNESS_DECREASE
  bldCnt = (1 << 3) | (3 << 6);
  bldY = intensity & 0xF;
}

void disableColorEffects() { bldCnt = 0; }

void setForegroundOpacitySub(s8 opacity) {
  // ALPHA | BG3
  bldCntSub = (1 << 6) | (1 << 11);

  if (opacity > 16)
    opacity = 16;
  bldAlphaSub = (opacity & 0xF) | (((16 - opacity) & 0xF) << 8);
}

void setDarkenBackgroundSub(s8 intensity) {
  // BG3 | BRIGHTNESS_DECREASE
  bldCntSub = (1 << 3) | (3 << 6);
  bldYSub = intensity & 0xF;
}

void disableColorEffectsSub() { bldCntSub = 0; }
} // namespace Engine
