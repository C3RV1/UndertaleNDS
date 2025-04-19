#include "Battle/BattleAction.hpp"

void BattleAction::drawTarget() {
  // TODO: Do not show spared nor killed enemies
  if (_cTarget >= (*_enemies).size())
    _cTarget = (*_enemies).size() - 1;
  if (_cTarget < 0)
    _cTarget = 0;

  int enemyPageCount = (*_enemies).size() - (_cTarget / 4) * 4;
  const int enemyNameX = 100, enemySpacing = 20;
  int enemyNameY = 120 - (enemySpacing * (enemyPageCount + 1) / 2);
  _smallHeartSpr->_wx = (enemyNameX - 15) << 8;
  _smallHeartSpr->_wy = (enemyNameY + enemySpacing * (_cTarget % 4) + 4) << 8;

  if (_cTarget / 4 == _cPage)
    return;
  _cPage = _cTarget / 4;

  for (u32 i = 0, enemyId = _cPage * 4; i < 4 && enemyId < (*_enemies).size();
       i++, enemyId++) {
    // if (enemies[enemyId].spared || enemies[enemyId].hp <= 0)
    //     continue;
    int x = enemyNameX;
    int y = enemyNameY + i * enemySpacing;
    if ((*_enemies)[enemyId]->canBeSpared())
      Engine::textMain.setColor(12);
    else
      Engine::textMain.setColor(15);
    Engine::textMain.drawGlyph(_fnt, '*', x, y);
    Engine::textMain.drawGlyph(_fnt, ' ', x, y);
    for (auto const &c : (*_enemies)[enemyId]->getName()) {
      Engine::textMain.drawGlyph(_fnt, c, x, y);
    }
  }
}

bool BattleAction::updateChoosingTarget() {
  if (keysDown() & KEY_DOWN)
    _cTarget += 1;
  else if (keysDown() & KEY_UP)
    _cTarget -= 1;
  else if (keysDown() & KEY_RIGHT)
    _cTarget += 4;
  else if (keysDown() & KEY_LEFT)
    _cTarget -= 4;
  drawTarget();
  if (keysDown() & KEY_B) {
    switch (_cAction) {
    case ACTION_MERCY:
      enter(CHOOSING_MERCY);
      break;
    case ACTION_ACT:
    case ACTION_FIGHT:
      enter(CHOOSING_ACTION);
      break;
    }
  } else if (keysDown() & KEY_A) {
    switch (_cAction) {
    case ACTION_MERCY:
      return true;
      break;
    case ACTION_FIGHT:
      enter(FIGHTING);
      break;
    case ACTION_ACT:
      enter(CHOOSING_ACT);
      break;
    }
  }
  return false;
}
