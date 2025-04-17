#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemy.hpp"
#include "Engine/Texture.hpp"
#include "Save.hpp"

Dummy::Dummy() : Enemy() {
  loadName(1);

  _maxHp = 15;
  _hp = _maxHp;
  _actOptionCount = 2;
  dummySpr.loadTexture(
      Engine::textureManager.loadTexture("battle/dummy_ruins"));
  dummySpr._wx = 30 << 8;
  dummySpr._wy = ((192 - 52) / 2) << 8;

  loadActText(0);
}

void Dummy::doAct(int actId) { globalSave.flags[220] = actId; }

Engine::Sprite *Dummy::getSprite(u8 spriteId) { return &dummySpr; }

bool Dummy::canBeSpared() { return true; }

bool Dummy::damageAnimation() { return true; }

void Dummy::doDamage(int damage) { Enemy::doDamage(damage); }
