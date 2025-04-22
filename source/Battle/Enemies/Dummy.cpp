#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemy.hpp"
#include "Engine/Sprite.hpp"
#include <memory>
#include <string>

Dummy::Dummy() : Enemy() {
  loadName(1);

  _maxHp = 15;
  _hp = _maxHp;
  _actOptionCount = 2;
  _dummySpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  Engine::spriteLoadTexture(_dummySpr, "battle/dummy_ruins");
  _dummySpr->_wx = kX << 8;
  _dummySpr->_wy = kY << 8;
  _dummySpr->_layer = 1;
  Engine::spriteSetShown(_dummySpr, true);

  loadActText(0);
}

std::shared_ptr<Engine::Sprite> Dummy::getSprite(u8 spriteId) {
  return _dummySpr;
}

bool Dummy::canBeSpared() { return true; }

bool Dummy::damageAnimation() {
  if (_dummySpr->_texture == nullptr)
    return true;
  return defaultDamageAnimation(kX << 8, kY << 8,
                                _dummySpr->_texture->getWidth(),
                                _dummySpr->_texture->getHeight());
}

void Dummy::shakeSprites(s32 dx) { _dummySpr->_wx = (kX << 8) + dx; }

void Dummy::slashFinished() {
  Enemy::slashFinished();
  _dummySpr->setAnimation(_dummySpr->nameToAnimId("hurt"));
}
