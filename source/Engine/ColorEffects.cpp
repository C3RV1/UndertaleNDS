#include "Engine/ColorEffects.hpp"

namespace Engine {
u32 bldCnt = 0, bldAlpha = 0, bldY = 0;
u32 bldCntSub = 0, bldAlphaSub = 0, bldYSub = 0;
u32 masterBright = 0, masterBrightSub = 0;

void setForegroundOpacity(u8 opacity) {
  // ALPHA | BG3
  bldCnt = (1 << 6) | (1 << 11);

  if (opacity > 16)
    opacity = 16;
  bldAlpha = (opacity & 0xF) | (((16 - opacity) & 0xF) << 8);
}

void setDarkenBackground(u8 intensity) {
  // BG3 | BRIGHTNESS_DECREASE
  bldCnt = (1 << 3) | (3 << 6);
  bldY = intensity & 0xF;
}

void disableColorEffects() { bldCnt = 0; }

void setForegroundOpacitySub(u8 opacity) {
  // ALPHA | BG3
  bldCntSub = (1 << 6) | (1 << 11);

  if (opacity > 16)
    opacity = 16;
  bldAlphaSub = (opacity & 0xF) | (((16 - opacity) & 0xF) << 8);
}

void setDarkenBackgroundSub(u8 intensity) {
  // BG3 | BRIGHTNESS_DECREASE
  bldCntSub = (1 << 3) | (3 << 6);
  bldYSub = intensity & 0xF;
}

void disableColorEffectsSub() { bldCntSub = 0; }

void setScreenBrightness(s8 brightness) {
  if (brightness > 0)
    masterBright = (1 << 14) | (u8)brightness;
  else if (brightness == 0)
    masterBright = 0;
  else
    masterBright = (2 << 14) | (u8)(-brightness);
}

void setScreenBrightnessSub(s8 brightness) {
  if (brightness > 0)
    masterBrightSub = (1 << 14) | (u8)brightness;
  else if (brightness == 0)
    masterBrightSub = 0;
  else
    masterBrightSub = (2 << 14) | (u8)(-brightness);
}
} // namespace Engine
