#include "Battle/BattleAction.hpp"
#include "Engine/OAMManager.hpp"

void BattleAction::drawAct(bool draw) {
  constexpr int optionX = 50, optionY = 110, optionSpacingX = 90,
                optionSpacingY = 20;
  constexpr int offsetX = -15, offsetY = 4;
  if (draw) {
    Engine::textMain.setColor(15);
  }
  if (_cAct < 0)
    _cAct = 0;
  if (_cAct >= (*_enemies)[_cTarget]->getActOptionCount())
    _cAct = (*_enemies)[_cTarget]->getActOptionCount() - 1;
  if (_cAct < 0) {
    Engine::spriteSetShown(_smallHeartSpr, false);
    return;
  }
  Engine::spriteSetShown(_smallHeartSpr, true);
  _smallHeartSpr->_wx = (optionX + optionSpacingX * (_cAct % 2) + offsetX) << 8;
  _smallHeartSpr->_wy = (optionY + optionSpacingY * (_cAct / 2) + offsetY) << 8;
  if (!draw)
    return;
  int x = optionX, y = optionY;
  for (auto const &c : (*_enemies)[_cTarget]->getActText()) {
    if (c == '\n') {
      x = optionX;
      y += optionSpacingY;
      continue;
    }
    Engine::textMain.drawGlyph(_fnt, c, x, y);
  }
}

bool BattleAction::updateChoosingAct() {
  if (keysDown() & KEY_DOWN)
    _cAct += 2;
  else if (keysDown() & KEY_UP)
    _cAct -= 2;
  else if (keysDown() & KEY_RIGHT)
    _cAct += 1;
  else if (keysDown() & KEY_LEFT)
    _cAct -= 1;
  drawAct(false);
  if (keysDown() & KEY_B)
    enter(CHOOSING_TARGET);
  if (keysDown() & KEY_A)
    return true;
  return false;
}
