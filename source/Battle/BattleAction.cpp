//
// Created by cervi on 02/10/2022.
//

#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"

BattleAction::BattleAction(u8 enemyCount_, Enemy* enemies) :
            enemyCount(enemyCount_), enemies(enemies),
            fightBtn(Engine::Allocated3D),
            actBtn(Engine::Allocated3D),
            itemBtn(Engine::Allocated3D),
            mercyBtn(Engine::Allocated3D),
            heartSpr(Engine::Allocated3D)
{
    fnt.loadPath("fnt_maintext.font");

    fightTex.loadPath("btn/spr_fightbt");
    fightBtn.loadTexture(fightTex);
    fightBtn.wx = 12 << 8; fightBtn.wy = 36 << 8;

    actTex.loadPath("btn/spr_talkbt");
    actBtn.loadTexture(actTex);
    actBtn.wx = 134 << 8; actBtn.wy = 36 << 8;

    itemTex.loadPath("btn/spr_itembt");
    itemBtn.loadTexture(itemTex);
    itemBtn.wx = 12 << 8; itemBtn.wy = 114 << 8;

    mercyTex.loadPath("btn/spr_sparebt");
    mercyBtn.loadTexture(mercyTex);
    mercyBtn.wx = 134 << 8; mercyBtn.wy = 114 << 8;

    gfxAnimId = fightBtn.nameToAnimId("gfx");
    activeAnimId = fightBtn.nameToAnimId("active");

    heartTex.loadPath("spr_heart");
    heartSpr.loadTexture(heartTex);
    heartSpr.setShown(true);
    heartSpr.layer = 3;

    enter(CHOOSING_ACTION);
}

void BattleAction::enter(BattleActionState state) {
    currentState = state;
    if (state == CHOOSING_ACTION) {
        currentAction = ACTION_FIGHT;
        fightBtn.setShown(true);
        actBtn.setShown(true);
        itemBtn.setShown(true);
        mercyBtn.setShown(true);
        setBtn();
    }
}

void BattleAction::setBtn() {
    Engine::Sprite* activeSprite = nullptr;
    fightBtn.setSpriteAnim(gfxAnimId);
    actBtn.setSpriteAnim(gfxAnimId);
    itemBtn.setSpriteAnim(gfxAnimId);
    mercyBtn.setSpriteAnim(gfxAnimId);

    switch (currentAction) {
        case ACTION_FIGHT:
            activeSprite = &fightBtn;
            break;
        case ACTION_ACT:
            activeSprite = &actBtn;
            break;
        case ACTION_ITEM:
            activeSprite = &itemBtn;
            break;
        case ACTION_MERCY:
            activeSprite = &mercyBtn;
            break;
    }

    if (activeSprite != nullptr) {
        activeSprite->setSpriteAnim(activeAnimId);
        heartSpr.wx = activeSprite->wx + (8 << 8);
        heartSpr.wy = activeSprite->wy + (13 << 8);
    }
}

bool BattleAction::update() {
    switch (currentState) {
        case CHOOSING_ACTION:
            return updateChoosingAction();
        case CHOOSING_TARGET:
            return updateChoosingTarget();
        case CHOOSING_ACT:
            return updateChoosingAct();
        case CHOOSING_MERCY:
            return updateChoosingMercy();
    }
    return true;
}

bool BattleAction::updateChoosingAction() {
    int prevAction = currentAction;
    if (keysDown() & KEY_LEFT)
        currentAction &= ~1; // FIGHT (0) or ITEM (2)
    else if (keysDown() & KEY_RIGHT)
        currentAction |= 1; // ACT (1) or MERCY (3)
    else if (keysDown() & KEY_UP)
        currentAction &= ~2; // FIGHT (0) or ACT (1)
    else if (keysDown() & KEY_DOWN)
        currentAction |= 2; // ITEM (2) or MERCY (3)

    if (prevAction != currentAction)
        setBtn();
    return false;
}

bool BattleAction::updateChoosingTarget() {
    return false;
}

bool BattleAction::updateChoosingAct() {
    return false;
}

bool BattleAction::updateChoosingMercy() {
    return false;
}
