#include "Battle/BattleAction.hpp"

bool BattleAction::updateFighting() {
  // TODO: Show damage

  Enemy *enemy = (*_enemies)[_cTarget].get();
  if (keysDown() & (KEY_A | KEY_TOUCH)) {
    s32 distanceFromCenter = _attackSpr._wx - (128 << 8);
    if (distanceFromCenter < 0)
      distanceFromCenter = -distanceFromCenter;
    s16 damage;
    u8 weaponAtk = 0; // TODO: get attack from current weapon
    u8 atk = 10 + weaponAtk;
    double randValue = ((double)rand() / (double)RAND_MAX) * 2.0;
    if (distanceFromCenter < 24 << 8) {
      // round((atk - def + rand(2)) * 2.2)
      damage = (s16)((atk - enemy->getDefense() + randValue) * 22.0 / 10.0);
    } else {
      // round((atk - def + rand(2)) * (1 - distance from center/target width) *
      // 2)
      double distanceFactor =
          1.0 - ((double)(distanceFromCenter >> 8) - 24) / (128.0 - 24.0);
      damage =
          (s16)((atk - enemy->getDefense() + randValue) * distanceFactor * 2.0);
    }
    if (damage < 0)
      damage = 0;

    char buffer[200];
    sprintf(buffer, "Damage %d\n", damage);
    nocashMessage(buffer);

    enemy->doDamage(damage);
    enemy->_hp -= damage;
    if (enemy->_hp < 0)
      enemy->_hp = 0;
    enter(SHOWING_DAMAGE);
    return false;
  }
  if (_attackSpr._wx > 256 << 8) {
    enemy->doDamage(0);
    enter(SHOWING_DAMAGE);
    return false;
  }
  _attackSpr._wx += kAttackSpeed;
  return false;
}
