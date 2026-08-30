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

enum class FadeScreen {
  NONE = 0,
  MAIN = 0b1,
  SUB = 0b10,
  BOTH = 0b11
};

class Fader {
public:
  void startFade(FadeType fadeType, FadeScreen screen);
  FadeType getLastFadeType() const;
  void update();
  bool fadeFinished() const;
private:
  FadeType _cFadeType = FadeType::NO_FADE;
  FadeScreen _cFadeScreen = FadeScreen::NONE;
  int _cTimer = 0;
};

#endif
