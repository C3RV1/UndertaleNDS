//
// Created by cervi on 02/10/2022.
//

#include "Battle/BattleAction.hpp"
#include "Engine/Engine.hpp"

#include "Engine/Background.hpp"
#include "Engine/Font.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Engine/Texture.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"
#include <memory>

// TODO: Touchscreen

BattleAction::BattleAction(std::vector<std::unique_ptr<Enemy>> *enemies,
                           int flavorTextId)
    : _enemies(enemies) {
  _bigHeartSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  _smallHeartSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  _attackSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  _fnt = Engine::fontManager.loadFont("fnt_maintext.font");
  _selectSnd.load("snd_select.wav");

  for (int i = 0; i < 4; i++) {
    _btn[i] = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
    _btn[i]->_wx = _buttonPositions[i][0];
    _btn[i]->_wy = _buttonPositions[i][1];
    Engine::spriteSetShown(_btn[i], true);
  }

  Engine::spriteLoadTexture(_btn[ACTION_FIGHT], "btn/spr_fightbt", true);
  Engine::spriteLoadTexture(_btn[ACTION_ACT], "btn/spr_talkbt", true);
  Engine::spriteLoadTexture(_btn[ACTION_ITEM], "btn/spr_itembt", true);
  Engine::spriteLoadTexture(_btn[ACTION_MERCY], "btn/spr_sparebt", true);

  _gfxAnimId = _btn[0]->nameToAnimId("gfx");
  _activeAnimId = _btn[0]->nameToAnimId("active");

  Engine::spriteLoadTexture(_bigHeartSpr, "spr_heart");
  Engine::spriteSetShown(_bigHeartSpr, false);
  _bigHeartSpr->_layer = 3;

  Engine::spriteLoadTexture(_smallHeartSpr, "spr_heartsmall");
  _smallHeartSpr->_layer = 3;

  FILE *f = fopen("nitro:/data/mercy.txt", "rb");
  if (!f)
    Engine::throw_("Error opening mercy text");
  int len = str_len_file(f, '@');
  _mercyText.resize(len);
  fread(&_mercyText[0], len, 1, f);
  fseek(f, 1, SEEK_CUR);
  fclose(f);

  if (flavorTextId == -1) {
    enter(CHOOSING_ACTION);
    return;
  }
  std::string buffer =
      "nitro:/data/flavorTexts/" + std::to_string(flavorTextId) + ".txt";
  f = fopen(buffer.c_str(), "rb");
  if (!f) {
    Engine::throw_("Error opening flavor text " + std::to_string(flavorTextId));
  }
  len = str_len_file(f, '\0');
  _flavorText.resize(len);
  fread(&_flavorText[0], len, 1, f);
  // fseek(f, 1, SEEK_CUR); FIXME: not needed right?
  fclose(f);

  enter(PRINTING_FLAVOR_TEXT);
}

void BattleAction::clearReuse() {
  Engine::textureManager.clearEnsureReuse("btn/spr_fightbt");
  Engine::textureManager.clearEnsureReuse("btn/spr_talkbt");
  Engine::textureManager.clearEnsureReuse("btn/spr_itembt");
  Engine::textureManager.clearEnsureReuse("btn/spr_sparebt");
}

void BattleAction::enter(BattleActionState state) {
  _cState = state;
  Engine::spriteSetShown(_bigHeartSpr, state == CHOOSING_ACTION);
  Engine::spriteSetShown(_smallHeartSpr, state != PRINTING_FLAVOR_TEXT &&
                                             state != CHOOSING_ACTION &&
                                             state != FIGHTING &&
                                             state != SHOWING_DAMAGE);
  Engine::textMain.clearRect(0, 192 / 2, 256, 192 / 2);
  switch (state) {
  case PRINTING_FLAVOR_TEXT:
    _flavorTextDialogue = std::make_unique<FlavorTextDialogue>(_flavorText);
    _flavorTextDialogue->setShown(true);
    break;
  case CHOOSING_ACTION:
    setBtn();
    break;
  case CHOOSING_TARGET:
    _cTarget = 0;
    _cPage = -1;
    drawTarget();
    break;
  case CHOOSING_ACT:
    _cAct = 0;
    drawAct(true);
    break;
  case CHOOSING_MERCY:
    _mercyFlee = false;
    drawMercy(true);
    break;
  case FIGHTING:
    _fightBoard.loadPath("fight_board");
    _fightBoard.loadBgTextMain();
    Engine::spriteLoadTexture(_attackSpr, "battle/spr_targetchoice");
    Engine::spriteSetShown(_attackSpr, true);
    _attackSpr->_wx = 0;
    _attackSpr->_wy = ((192 * 3 / 2 - _attackSpr->_texture->getHeight()) / 2)
                      << 8;
    break;
  case CHOOSING_ITEM:
    Engine::spriteSetShown(_smallHeartSpr, false);
    break;
  case SHOWING_DAMAGE:
    _attackSpr->setAnimation(_attackSpr->nameToAnimId("flashing"));
    break;
  }
}

void BattleAction::setBtn() {
  for (auto &btn : _btn) {
    Engine::spriteSetShown(btn, true);
    btn->setAnimation(_gfxAnimId);
  }

  _btn[_cAction]->setAnimation(_activeAnimId);
  _bigHeartSpr->_wx = _btn[_cAction]->_wx + (8 << 8);
  _bigHeartSpr->_wy = _btn[_cAction]->_wy + (13 << 8);
}

bool BattleAction::update() {
  switch (_cState) {
  case CHOOSING_ACTION:
    return updateChoosingAction();
  case CHOOSING_TARGET:
    return updateChoosingTarget();
  case CHOOSING_ACT:
    return updateChoosingAct();
  case CHOOSING_MERCY:
    return updateChoosingMercy();
  case CHOOSING_ITEM:
    return updateChoosingItem();
  case FIGHTING:
    return updateFighting();
  case PRINTING_FLAVOR_TEXT:
    return updatePrintingFlavor();
  case SHOWING_DAMAGE:
    return updateShowingDamage();
  }
  return true;
}

int BattleAction::getActionNum() const {
  switch (_cAction) {
  case ACTION_FIGHT:
    return 0 + _cTarget;
  case ACTION_ACT:
    return 10 + _cTarget * 4 + _cAct;
  case ACTION_ITEM:
    return 60;
  case ACTION_MERCY:
    if (_mercyFlee)
      return 40;
    return 41 + _cTarget;
  default:
    nocashMessage("GetActionNum fail");
    return 0;
  }
}

BattleAction::~BattleAction() {
  globalSave.flags[FlagIds::BATTLE_ACTION] = getActionNum();
}
