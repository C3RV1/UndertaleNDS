#include "Battle/Enemies/Froggit.hpp"
#include "Engine/Sprite.hpp"
#include <memory>

Froggit::Froggit(bool isFirstEnemy) {
  loadName(2);

  if (isFirstEnemy)
    _maxHp = 20;
  else
    _maxHp = 30;

  _hp = _maxHp;

  _legsSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  _headSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);

  Engine::spriteLoadTexture(_legsSpr, "battle/spr_froglegs");
  Engine::spriteLoadTexture(_headSpr, "battle/spr_froghead");

  _legsSpr->_wx = kX << 8;
  _legsSpr->_wy = kY << 8;
  _legsSpr->_layer = 1;
  Engine::spriteSetShown(_legsSpr, true);

  _headSpr->_wx = kX << 8;
  _headSpr->_wy = kY << 8;
  _headSpr->_layer = 1;
  Engine::spriteSetShown(_headSpr, true);

  loadActText(1);
  _actOptionCount = 3;
}

std::shared_ptr<Engine::Sprite> Froggit::getSprite(u8 spriteId) {
  if (spriteId == 0)
    return _headSpr;
  return _legsSpr;
}

void Froggit::update() {
  auto pos = _headPath.advance(kPathSpeed);
  _headSpr->_wx = (kX << 8) + pos.first;
  _headSpr->_wy = (kY << 8) + pos.second;
}

bool Froggit::damageAnimation() {
  if (_headSpr->_texture == nullptr)
    return true;
  return defaultDamageAnimation(kX << 8, kY << 8,
                                _headSpr->_texture->getWidth(),
                                _headSpr->_texture->getHeight());
}

bool Froggit::canBeSpared() { return true; }

void Froggit::doAct(int actId) {}

void Froggit::slashFinished() {}

void Froggit::damageAnimationFinished() {}
