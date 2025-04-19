#include "Battle/Battle.hpp"
#include "Engine/OAMManager.hpp"
#include "Engine/TextBGManager.hpp"
#include "Engine/math.hpp"

bool Battle::moveInBattleRect() {
  if (_moveCounter >= kMoveFrames) {
    drawRect();
    return true;
  }
  Engine::spriteSetShown(_playerSpr, false);
  int x, y, w, h;
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawHollowRect(
      x - kFlavorRectWidth, y - kFlavorRectWidth, w + kFlavorRectWidth * 2,
      h + kFlavorRectWidth * 2, kFlavorRectWidth, 0);
  _moveCounter++;
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawHollowRect(
      x - kFlavorRectWidth, y - kFlavorRectWidth, w + kFlavorRectWidth * 2,
      h + kFlavorRectWidth * 2, kFlavorRectWidth, 15);
  return _moveCounter >= kMoveFrames;
}

bool Battle::moveOutBattleRect() {
  if (_moveCounter == 0) {
    drawRect();
    return true;
  }
  int x, y, w, h;
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawHollowRect(
      x - kFlavorRectWidth, y - kFlavorRectWidth, w + kFlavorRectWidth * 2,
      h + kFlavorRectWidth * 2, kFlavorRectWidth, 0);
  _moveCounter--;
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawHollowRect(
      x - kFlavorRectWidth, y - kFlavorRectWidth, w + kFlavorRectWidth * 2,
      h + kFlavorRectWidth * 2, kFlavorRectWidth, 15);
  return _moveCounter == 0;
}

void Battle::getMoveRect(int &x, int &y, int &w, int &h, int counter,
                         int maxCounter) {
  x = lerp(globalBattle->_boardX, kFlavorRectX, counter, maxCounter);
  y = lerp(globalBattle->_boardY, kFlavorRectY, counter, maxCounter);
  w = lerp(globalBattle->_boardW, kFlavorRectW, counter, maxCounter);
  h = lerp(globalBattle->_boardH, kFlavorRectH, counter, maxCounter);
}

void Battle::drawRect() {
  int x, y, w, h;
  Engine::spriteSetShown(_playerSpr, _moveCounter == 0);
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawHollowRect(
      x - kFlavorRectWidth, y - kFlavorRectWidth, w + kFlavorRectWidth * 2,
      h + kFlavorRectWidth * 2, kFlavorRectWidth, 15);
}

void Battle::clearRectInside() {
  int x, y, w, h;
  getMoveRect(x, y, w, h, _moveCounter, kMoveFrames);
  Engine::textMain.drawRect(x, y, w, h, 0);
}

void Battle::drawBulletBoard() {
  _bulletBoard.loadBgTextMain();
  drawRect();
  clearRectInside();
}
