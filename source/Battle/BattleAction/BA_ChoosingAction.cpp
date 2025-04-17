#include "Battle/BattleAction.hpp"

bool BattleAction::updateChoosingAction() {
  int prevAction = _cAction;
  if (keysDown() & KEY_LEFT)
    _cAction &= ~1; // FIGHT (0) or ITEM (2)
  else if (keysDown() & KEY_RIGHT)
    _cAction |= 1; // ACT (1) or MERCY (3)
  else if (keysDown() & KEY_UP)
    _cAction &= ~2; // FIGHT (0) or ACT (1)
  else if (keysDown() & KEY_DOWN)
    _cAction |= 2; // ITEM (2) or MERCY (3)

  if (prevAction != _cAction)
    setBtn();

  if (keysDown() & KEY_A) {
    for (int i = 0; i < 4; i++) {
      _btn[i].setShown(false);
    }
    switch (_cAction) {
    case ACTION_FIGHT:
    case ACTION_ACT:
      enter(CHOOSING_TARGET);
      break;
    case ACTION_MERCY:
      enter(CHOOSING_MERCY);
      break;
    case ACTION_ITEM:
      enter(CHOOSING_ITEM);
      break;
    }
  }
  return false;
}
