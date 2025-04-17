#include "Battle/BattleAction.hpp"

bool BattleAction::updateShowingDamage() {
  Enemy *enemy = (*_enemies)[_cTarget].get();
  return enemy->damageAnimation();
}
