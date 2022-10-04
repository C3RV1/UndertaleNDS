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
    heartSpr.layer = 3;

    enter(CHOOSING_ACTION);
}

void BattleAction::enter(BattleActionState state) {
    currentState = state;
    fightBtn.setShown(state == CHOOSING_ACTION);
    actBtn.setShown(state == CHOOSING_ACTION);
    itemBtn.setShown(state == CHOOSING_ACTION);
    mercyBtn.setShown(state == CHOOSING_ACTION);
    heartSpr.setShown(state == CHOOSING_ACTION);
    switch (state) {
        case CHOOSING_ACTION:
            currentAction = ACTION_FIGHT;
            Engine::textMain.clear();
            setBtn();
            break;
        case CHOOSING_TARGET:
            chosenTarget = 0;
            currentPage = -1;
            updatePageTarget();
            break;
    }
}

void BattleAction::updatePageTarget() {
    if (chosenTarget >= enemyCount)
        chosenTarget = enemyCount - 1;
    if (chosenTarget < 0)
        chosenTarget = 0;
    if (chosenTarget / 4 == currentPage)
        return;
    currentPage = chosenTarget / 4;
    int enemyPageCount = enemyCount - currentPage * 4;
    const int enemyNameX = 100, enemySpacing = 20;
    int enemyNameY = 96 - (enemySpacing * (enemyPageCount + 1) / 2);
    Engine::textMain.clear();
    for (int i = 0, enemyId = currentPage * 4; i < 4 && enemyId < enemyCount; i++, enemyId++) {
        int x = enemyNameX;
        int y = enemyNameY + i * enemySpacing;
        if (enemyId == chosenTarget)
            Engine::textMain.setCurrentColor(12);
        else
            Engine::textMain.setCurrentColor(15);
        Engine::textMain.drawGlyph(fnt, '*', x, y);
        Engine::textMain.drawGlyph(fnt, ' ', x, y);
        for (char *p = enemies[enemyId].enemyName; *p != 0; p++) {
            Engine::textMain.drawGlyph(fnt, *p, x, y);
        }
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
        case CHOOSING_ITEM:
            return updateChoosingItem();
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

    if (keysDown() & KEY_A) {
        switch (currentAction) {
            case ACTION_FIGHT:
            case ACTION_ACT:
                enter(CHOOSING_TARGET);
                break;
            case ACTION_ITEM:
                enter(CHOOSING_ITEM);
            case ACTION_MERCY:
                enter(CHOOSING_MERCY);
        }
    }
    return false;
}

bool BattleAction::updateChoosingTarget() {
    if (keysDown() & KEY_B) {
        switch (currentAction) {
            case ACTION_MERCY:
                enter(CHOOSING_MERCY);
                break;
            case ACTION_ACT:
            case ACTION_FIGHT:
                enter(CHOOSING_ACTION);
                break;
        }
    }
    else if (keysDown() & KEY_A) {
        switch (currentAction) {
            case ACTION_MERCY:
            case ACTION_FIGHT:
                return true;
            case ACTION_ACT:
                enter(CHOOSING_ACT);
                break;
        }
    }
    else if (keysDown() & KEY_DOWN)
        chosenTarget += 1;
    else if (keysDown() & KEY_UP)
        chosenTarget -= 1;
    else if (keysDown() & KEY_RIGHT)
        chosenTarget += 4;
    else if (keysDown() & KEY_LEFT)
        chosenTarget -= 4;
    updatePageTarget();
    return false;
}

bool BattleAction::updateChoosingAct() {
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    return false;
}

bool BattleAction::updateChoosingMercy() {
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    return false;
}

bool BattleAction::updateChoosingItem() {
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    return false;
}
