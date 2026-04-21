#include "Battle/BattleAction.hpp"
#include "Engine/OAMManager.hpp"

void BattleAction::drawMercy(bool draw) {
  const int optionX = 100, optionY = 110, optionSpacingY = 20;
  const int offsetX = -15, offsetY = 4;
  if (draw) {
    Engine::textMain.setColor(15);
  }
  Engine::spriteSetShown(_smallHeartSpr, true);
  _smallHeartSpr->_wx = (optionX + offsetX) << 8;
  _smallHeartSpr->_wy = (optionY + optionSpacingY * _mercyFlee + offsetY) << 8;
  if (!draw)
    return;
  int x = optionX, y = optionY;
  for (auto const &c : _mercyText) {
    if (c == '\n') {
      x = optionX;
      y += optionSpacingY;
      continue;
    }
    Engine::textMain.drawGlyph(*_fnt, c, x, y);
  }
}

bool BattleAction::updateChoosingMercy() {
  if (keysDown() & KEY_DOWN)
    _mercyFlee = true;
  else if (keysDown() & KEY_UP)
    _mercyFlee = false;
  drawMercy(false);
  if (keysDown() & KEY_B) {
    enter(CHOOSING_ACTION);
  } else if (keysDown() & KEY_A) {
    if (_mercyFlee)
      return true;
    else
      enter(CHOOSING_TARGET);
  }
  return false;
}
