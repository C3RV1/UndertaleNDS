#include "Fader.hpp"
#include "nds.h"
#include "Engine/math.hpp"

void Fader::startFade(FadeType fadeType) {
  _cFadeType = fadeType;
  _cTimer = 0;
}

FadeType Fader::getLastFadeType() const {
  return _cFadeType;
}

void Fader::update() {
  if (_cTimer >= kFadeFrames || _cFadeType == FadeType::NO_FADE)
    return;
  switch(_cFadeType) {
  case FadeType::FADE_OUT:
    setBrightness(1, lerp(0, -16, _cTimer++, kFadeFrames));
    break;
  case FadeType::FADE_IN:
    setBrightness(1, lerp(-16, 0, _cTimer++, kFadeFrames));
    break;
  default:
    break;
  }
}

bool Fader::fadeFinished() const {
  return _cTimer >= kFadeFrames || _cFadeType == FadeType::NO_FADE;
}
