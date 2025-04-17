#include "Battle/BattleAction.hpp"

bool BattleAction::updateChoosingItem() {
  if (keysDown() & KEY_B)
    enter(CHOOSING_ACTION);
  return false;
}
