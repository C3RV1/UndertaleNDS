//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_BATTLE_HPP
#define UNDERTALE_BATTLE_HPP

#include "BattleAction.hpp"
#include "BattleAttack.hpp"
#include "BattleNavigation.hpp"
#include "Enemy.hpp"
#include "Formats/utils.hpp"
#include "Fader.hpp"
#include <memory>
#include <nds.h>

class Cutscene;
class SaveData;

class Battle {
public:
  explicit Battle(Cutscene* cutscene);
  void exit(bool won);
  void loadFromBuffer(BufferReader& br);
  void showHp();
  void enter();
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
  BattleNavigation _nav;

  std::vector<std::unique_ptr<Enemy>> _enemies;

  std::vector<std::shared_ptr<Engine::Sprite>> _sprites;

  Engine::Background _bulletBoard;
  u8 _boardX = 0, _boardY = 0, _boardW = 0, _boardH = 0;

  constexpr static s32 _playerSpeed = (60 << 8) / 60;
  std::shared_ptr<Engine::Sprite> _playerSpr;
  Engine::Background _battleBackground;
  std::shared_ptr<Engine::Font> _fnt;

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
  
private:
  Cutscene* _cutscene;
  
public:
  SaveData* _save;
};

#endif // UNDERTALE_BATTLE_HPP
