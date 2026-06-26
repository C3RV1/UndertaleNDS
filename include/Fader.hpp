//
// Created by cervi on 26/05/2026
//

#ifndef UNDERTALE_FADER_HPP
#define UNDERTALE_FADER_HPP

constexpr int kFadeFrames = 20;
enum class FadeType {
  NO_FADE,
  FADE_IN,
  FADE_OUT
};

class Fader {
public:
  void startFade(FadeType fadeType);
  FadeType getLastFadeType() const;
  void update();
  bool fadeFinished() const;
private:
  FadeType _cFadeType = FadeType::NO_FADE;
  int _cTimer = 0;
};

#endif
