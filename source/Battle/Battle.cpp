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
#include "Engine/Engine.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Formats/utils.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include "Save.hpp"
#include <cstdio>
#include <memory>
#include <string>

std::unique_ptr<Battle> globalBattle = nullptr;

Battle::Battle() {
  _fnt = Engine::fontManager.loadFont("fnt_curs.font");
  _playerSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_playerSpr, "spr_heartsmall");
  _playerSpr->_wx = ((256 - 16) / 2) << 8;
  _playerSpr->_wy = ((192 - 32) / 2) << 8;
  _playerSpr->_layer = 100;

  for (int i = 220; i <= 229; i++) {
    globalSave.flags[i] = 0;
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
    globalSave.exp += earnedExp;
    globalSave.gold += earnedGold;

    int size_s =
        std::snprintf(nullptr, 0, _winText.c_str(), earnedExp, earnedGold);
    std::string buffer;
    buffer.resize(size_s);
    sprintf(&buffer[0], _winText.c_str(), earnedExp, earnedGold);
    if (globalCutscene->_cDialogue == nullptr) {
      auto dialogue = std::make_unique<FlavorTextDialogue>(buffer);
      dialogue->setShown(true);
      globalCutscene->_cDialogue = std::move(dialogue);
    }
    _stopPostDialogue = true;
  } else {
    _running = false;
  }
}

void Battle::loadFromStream(FILE *stream) {
  u8 enemyCount;
  fread(&enemyCount, 1, 1, stream);
  _enemies.resize(enemyCount);
  _cBattleAttacks.resize(enemyCount);
  std::string buffer;
  u8 enemyId;
  for (int i = 0; i < enemyCount; i++) {
    fread(&enemyId, 1, 1, stream);
    _enemies[i] = getEnemy(enemyId);
  }

  u8 boardId;
  fread(&boardId, 1, 1, stream);
  buffer = "battle/board" + std::to_string(boardId);
  _bulletBoard.loadPath(buffer);

  fread(&_boardX, 1, 1, stream);
  fread(&_boardY, 1, 1, stream);
  fread(&_boardW, 1, 1, stream);
  fread(&_boardH, 1, 1, stream);

  bool boardIsFlavor;
  fread(&boardIsFlavor, 1, 1, stream);
  if (boardIsFlavor)
    _moveCounter = kMoveFrames;
  else
    _moveCounter = 0;

  int len = str_len_file(stream, '\0');
  std::string bgPath;
  bgPath.resize(len);
  fread(&bgPath[0], len, 1, stream);
  fseek(stream, 1, SEEK_CUR);
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
  Engine::textMain.drawHpBar(globalSave.hp, globalSave.maxHp, kHPx, kHPy, kHPw,
                             kHPh);

  Engine::textMain.setColor(15);

  char buffer[16];
  sprintf(buffer, "%2d/%2d", globalSave.hp, globalSave.maxHp);
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
