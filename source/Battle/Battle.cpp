//
// Created by cervi on 02/09/2022.
//

#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"
#include "Battle/Enemy.hpp"
#include "Battle/FlavorTextDialogue.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Background.hpp"
#include "Engine/DataBank.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Formats/utils.hpp"
#include "Room/Room.hpp"
#include <cstdio>
#include <memory>
#include <string>

Battle::Battle(Cutscene *cutscene)
    : _nav(this), _cutscene(cutscene), _save(cutscene->_room->_save.get()) {
  _fnt = Engine::fontManager.loadFont("fnt_curs.font");
  _playerSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_playerSpr, "spr_heartsmall");
  _playerSpr->_wx = ((256 - 16) / 2) << 8;
  _playerSpr->_wy = ((192 - 32) / 2) << 8;
  _playerSpr->_layer = 100;

  for (int i = 220; i <= 229; i++) {
    _save->flags[i] = 0;
  }

  _winText = textBank.getText("battle_win.txt");
}

void Battle::exit(bool won) {
  BattleAction::clearReuse();
  if (won) {
    hide();
    int earnedExp = 0, earnedGold = 0;
    for (auto &_enemy : _enemies) {
      if (_enemy->_hp <= 0)
        earnedExp += _enemy->_expOnKill;
      earnedGold += _enemy->_goldOnWin;
    }
    _save->exp += earnedExp;
    _save->gold += earnedGold;

    int size_s =
        std::snprintf(nullptr, 0, _winText.c_str(), earnedExp, earnedGold);
    std::string buffer;
    buffer.resize(size_s);
    sprintf(&buffer[0], _winText.c_str(), earnedExp, earnedGold);
    if (_cutscene->_cDialogue == nullptr) {
      auto dialogue = std::make_unique<FlavorTextDialogue>(this, buffer);
      dialogue->setShown(true);
      _cutscene->_cDialogue = std::move(dialogue);
    }
    _stopPostDialogue = true;
  } else {
    _running = false;
  }
}

void Battle::loadFromBuffer(BufferReader &br) {
  u8 enemyCount;
  br.read(&enemyCount, 1);
  _enemies.resize(enemyCount);
  _cBattleAttacks.resize(enemyCount);
  std::string buffer;
  u8 enemyId;
  for (int i = 0; i < enemyCount; i++) {
    br.read(&enemyId, 1);
    _enemies[i] = getEnemy(this, enemyId);
  }

  u8 boardId;
  br.read(&boardId, 1);
  buffer = "battle/board" + std::to_string(boardId);
  _bulletBoard.loadPath(buffer);

  br.read(&_boardX, 1);
  br.read(&_boardY, 1);
  br.read(&_boardW, 1);
  br.read(&_boardH, 1);

  bool boardIsFlavor;
  br.read(&boardIsFlavor, 1);
  if (boardIsFlavor)
    _moveCounter = kMoveFrames;
  else
    _moveCounter = 0;

  std::string bgPath = br.readstring();
  _battleBackground.loadPath(bgPath);

  _playerSpr->_wx = ((_boardX + _boardW / 2) << 8) - (9 << 8) / 2;
  _playerSpr->_wy = ((_boardY + _boardH / 2) << 8) - (9 << 8) / 2;
}

void Battle::enter() {
  _battleBackground.loadBgTextSub();
  for (auto &c : _enemies)
    c->enter();
  show();
}

void Battle::show() {
  _shown = true;
  showHp();
  Engine::clearMain();
  drawBulletBoard();
}

void Battle::showHp() {
  if (!_shown)
    return;
  // TODO: Maybe render hp with font, instead of baking to background?

  constexpr int kHPx = 108, kHPy = 154, kHPw = 18, kHPh = 14;
  constexpr int kPadding = 6, kTxtYOff = 0, kTotalWidth = 100;

  Engine::textMain.clearRect(kHPx, kHPy, kTotalWidth, kHPh);
  Engine::textMain.drawHpBar(_save->hp, _save->maxHp, kHPx, kHPy, kHPw,
                             kHPh);

  Engine::textMain.setColor(15);

  char buffer[16];
  sprintf(buffer, "%2d/%2d", _save->hp, _save->maxHp);
  int x = kHPx + kHPw + kPadding;
  for (char *p = buffer; *p != 0; p++)
    Engine::textMain.drawGlyph(*_fnt, *p, x, kHPy + kTxtYOff);
}

void Battle::hide() {
  Engine::clearMain();
  Engine::spriteSetShown(_playerSpr, false);
  _shown = false;
}

void Battle::startBattleAttacks() {
  for (u32 i = 0; i < _enemies.size(); i++) {
    Enemy *enemy = _enemies[i].get();
    if (!enemy->getSpared() && enemy->_hp > 0) {
      _cBattleAttacks[i] = enemy->getBattleAttack();
    }
  }
}

void Battle::updateBattleAttacks() {
  for (u32 i = 0; i < _enemies.size(); i++) {
    BattleAttack *btlAttack = _cBattleAttacks[i].get();
    if (btlAttack != nullptr) {
      if (!moveOutBattleRect())
        return;
      if (btlAttack->update()) {
        _cBattleAttacks[i].reset();
      }
    }
  }
}

void Battle::update() {
  if (!_running)
    return;
  _nav.update();
  updateBattleAttacks();
  updateEnemies();
  
  if (_cBattleAction != nullptr) {
    if (_cBattleAction->update()) {
      _cBattleAction = nullptr;
      show();
    } else {
      return;
    }
  }

  if (_stopPostDialogue && _cutscene->_cDialogue == nullptr) {
    _running = false;
    return;
  }
  
  if (!_shown)
    return;
  
  if (keysHeld() & KEY_RIGHT) {
    _playerSpr->_wx += _playerSpeed;
  }
  if (keysHeld() & KEY_LEFT) {
    _playerSpr->_wx -= _playerSpeed;
  }
  if (keysHeld() & KEY_DOWN) {
    _playerSpr->_wy += _playerSpeed;
  }
  if (keysHeld() & KEY_UP) {
    _playerSpr->_wy -= _playerSpeed;
  }
  if (keysHeld() & KEY_TOUCH) {
    touchPosition touchInfo;
    touchRead(&touchInfo);
    _playerSpr->_wx = (touchInfo.px << 8) - (9 << 8) / 2;
    _playerSpr->_wy = (touchInfo.py << 8) - (9 << 8) / 2;
  }

  if (_playerSpr->_wx < _boardX << 8) {
    _playerSpr->_wx = _boardX << 8;
  } else if (_playerSpr->_wx > (_boardX + _boardW - 9) << 8) {
    _playerSpr->_wx = (_boardX + _boardW - 9) << 8;
  }
  if (_playerSpr->_wy < _boardY << 8) {
    _playerSpr->_wy = _boardY << 8;
  } else if (_playerSpr->_wy > (_boardY + _boardH - 9) << 8) {
    _playerSpr->_wy = (_boardY + _boardH - 9) << 8;
  }
}

void Battle::updateEnemies() {
  for (auto &c : _enemies)
    c->update();
}
