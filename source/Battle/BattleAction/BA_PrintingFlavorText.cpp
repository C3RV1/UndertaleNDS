#include "Battle/BattleAction.hpp"

bool BattleAction::updatePrintingFlavor() {
  if (_flavorTextDialogue->update())
    enter(CHOOSING_ACTION);
  return false;
}
