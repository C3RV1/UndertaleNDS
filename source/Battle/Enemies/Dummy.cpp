#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemy.hpp"
#include "Engine/Texture.hpp"
#include "Save.hpp"

Dummy::Dummy() : Enemy() {
  loadName(1);

  _maxHp = 15;
  _hp = _maxHp;
  _actOptionCount = 2;
  _dummySpr.loadTexture(
      Engine::textureManager.loadTexture("battle/dummy_ruins"));
  _dummySpr._wx = kX << 8;
  _dummySpr._wy = kY << 8;
  _dummySpr._layer = 1;

  loadActText(0);
}

void Dummy::doAct(int actId) { globalSave.flags[220] = actId; }

Engine::Sprite *Dummy::getSprite(u8 spriteId) { return &_dummySpr; }

bool Dummy::canBeSpared() { return true; }

bool Dummy::damageAnimation() {
  if (_dummySpr._texture == nullptr)
    return true;
  return defaultDamageAnimation(kX << 8, kY << 8,
                                _dummySpr._texture->getWidth(),
                                _dummySpr._texture->getHeight());
}

void Dummy::doDamage(int damage) {
  Enemy::doDamage(damage);
  _dummySpr.setAnimation(_dummySpr.nameToAnimId("hurt"));
}
