#include "BootScreens/SplashScreen.hpp"
#include "Engine/Background.hpp"
#include "Engine/ColorEffects.hpp"
#include "Engine/Engine.hpp"
#include "Fader.hpp"

void runSplashScreen() {
  constexpr int kHoldFrames = 90;
  constexpr int kHoldFrames2 = 20;

  Engine::Background bg;
  bg.loadPath("blocks_ds_splash_screen");
  Engine::clearMain();
  Engine::clearSub();

  Fader fader;
  fader.startFade(FadeType::FADE_OUT, FadeScreen::MAIN);
  while (!fader.fadeFinished()) {
    fader.update();
    Engine::tick();
  }

  bg.loadBgTextMain();

  fader.startFade(FadeType::FADE_IN, FadeScreen::MAIN);
  while (!fader.fadeFinished()) {
    fader.update();
    Engine::tick();
  }

  for (int i = 0; i < kHoldFrames; i++)
    Engine::tick();

  fader.startFade(FadeType::FADE_OUT, FadeScreen::MAIN);
  while (!fader.fadeFinished()) {
    fader.update();
    Engine::tick();
  }

  Engine::clearMain();
  Engine::setScreenBrightness(0);
  for (int i = 0; i < kHoldFrames2; i++)
    Engine::tick();
}
