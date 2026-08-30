#include "Fader.hpp"
#include "Engine/ColorEffects.hpp"
#include "Engine/math.hpp"

void Fader::startFade(FadeType fadeType, FadeScreen screen) {
  _cFadeType = fadeType;
  _cFadeScreen = screen;
  _cTimer = 0;
}

FadeType Fader::getLastFadeType() const {
  return _cFadeType;
}

void Fader::update() {
  if (_cTimer > kFadeFrames || _cFadeType == FadeType::NO_FADE)
    return;
  switch(_cFadeType) {
  case FadeType::FADE_OUT:
    if ((int)_cFadeScreen & (int)FadeScreen::MAIN)
      Engine::setScreenBrightness(lerp(0, -16, _cTimer, kFadeFrames));
    if ((int)_cFadeScreen & (int)FadeScreen::SUB)
      Engine::setScreenBrightnessSub(lerp(0, -16, _cTimer, kFadeFrames));
    break;
  case FadeType::FADE_IN:
    if ((int)_cFadeScreen & (int)FadeScreen::MAIN)
      Engine::setScreenBrightness(lerp(-16, 0, _cTimer, kFadeFrames));
    if ((int)_cFadeScreen & (int)FadeScreen::SUB)
      Engine::setScreenBrightnessSub(lerp(-16, 0, _cTimer, kFadeFrames));
    break;
  default:
    break;
  }
  _cTimer++;
}

bool Fader::fadeFinished() const {
  return _cTimer > kFadeFrames || _cFadeType == FadeType::NO_FADE;
}
