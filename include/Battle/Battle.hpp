//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_BATTLE_HPP
#define UNDERTALE_BATTLE_HPP

#include "BattleAction.hpp"
#include "BattleAttack.hpp"
#include "Cutscene/Navigation.hpp"
#include "Enemy.hpp"
#include <cstdio>
#include <memory>
#include <nds.h>

// TODO: Display health

class Battle {
public:
  Battle();
  void exit(bool won);
  void loadFromStream(FILE *stream);
  void showHp();
  void show();
  void drawBulletBoard();
  void drawRect();
  void hide();
  void update();
  void updateBattleAttacks();
  void updateEnemies();
  void startBattleAttacks();
  void startRectMoveIn();
  void startRectMoveOut();
  void clearRectInside();
  bool _shown = false;
  bool _running = true;
  bool _stopPostDialogue = false;
  std::string _winText;
  Navigation _nav;

  std::vector<std::unique_ptr<Enemy>> _enemies;

  std::vector<std::shared_ptr<Engine::Sprite>> _sprites;

  Engine::Background _bulletBoard;
  u8 _boardX = 0, _boardY = 0, _boardW = 0, _boardH = 0;

  constexpr static s32 _playerSpeed = (60 << 8) / 60;
  std::shared_ptr<Engine::Sprite> _playerSpr;
  Engine::Background _battleBackground;
  Engine::Font _fnt;

  std::vector<std::unique_ptr<BattleAttack>> _cBattleAttacks;
  std::unique_ptr<BattleAction> _cBattleAction = nullptr;

  static constexpr int kFlavorRectX = 22, kFlavorRectY = 18;
  static constexpr int kFlavorRectW = 212, kFlavorRectH = 60;
  static constexpr int kFlavorRectWidth = 2;
  static constexpr int kMoveFrames = 12;
  u8 _moveCounter = 0;
  bool moveInBattleRect();
  bool moveOutBattleRect();
  void getMoveRect(int &x, int &y, int &w, int &h, int counter, int maxCounter);
};

void runBattle(FILE *stream);
extern std::unique_ptr<Battle> globalBattle;

#endif // UNDERTALE_BATTLE_HPP
