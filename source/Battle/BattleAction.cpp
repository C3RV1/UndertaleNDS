//
// Created by cervi on 02/10/2022.
//

#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"

// TODO: Touchscreen

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

    bigHeartTex.loadPath("spr_heart");
    smallHeartTex.loadPath("spr_heartsmall");
    heartSpr.layer = 3;

    FILE *f = fopen("nitro:/data/mercy.txt", "rb");
    if (f) {
        int len = str_len_file(f, '@');
        mercyText = new char[len + 1];
        fread(mercyText, len + 1, 1, f);
        mercyText[len] = '\0';
    } else {
        nocashMessage("Error opening mercy text");
    }
    fclose(f);

    enter(CHOOSING_ACTION);
}

void BattleAction::enter(BattleActionState state) {
    currentState = state;
    fightBtn.setShown(state == CHOOSING_ACTION);
    actBtn.setShown(state == CHOOSING_ACTION);
    itemBtn.setShown(state == CHOOSING_ACTION);
    mercyBtn.setShown(state == CHOOSING_ACTION);
    switch (state) {
        case CHOOSING_ACTION:
            currentAction = ACTION_FIGHT;
            Engine::textMain.clear();
            heartSpr.loadTexture(bigHeartTex);
            heartSpr.setShown(true);
            setBtn();
            break;
        case CHOOSING_TARGET:
            chosenTarget = 0;
            currentPage = -1;
            heartSpr.loadTexture(smallHeartTex);
            heartSpr.setShown(true);
            drawTarget();
            break;
        case CHOOSING_ACT:
            chosenAct = 0;
            heartSpr.loadTexture(smallHeartTex);
            heartSpr.setShown(true);
            drawAct(true);
            break;
        case CHOOSING_MERCY:
            mercyFlee = false;
            heartSpr.loadTexture(smallHeartTex);
            heartSpr.setShown(true);
            drawMercy(true);
            break;
    }
}

void BattleAction::drawAct(bool draw) {
    const int optionX = 40, optionY = 50, optionSpacingX = 90, optionSpacingY = 20;
    const int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.clear();
        Engine::textMain.setCurrentColor(15);
    }
    if (chosenAct < 0)
        chosenAct = 0;
    if (chosenAct >= enemies[chosenTarget].actOptionCount)
        chosenAct = enemies[chosenTarget].actOptionCount - 1;
    if (chosenAct < 0) {
        heartSpr.setShown(false);
        return;
    }
    heartSpr.setShown(true);
    heartSpr.wx = (optionX + optionSpacingX * (chosenAct % 2) + offsetX) << 8;
    heartSpr.wy = (optionY + optionSpacingY * (chosenAct / 2) + offsetY) << 8;
    if (!draw)
        return;
    if (enemies[chosenTarget].actText == nullptr)
        return;
    int x = optionX, y = optionY;
    for (char* p = enemies[chosenTarget].actText; *p != 0; p++) {
        if (*p == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(fnt, *p, x, y);
    }
}

void BattleAction::drawMercy(bool draw) {
    const int optionX = 100, optionY = 66, optionSpacingY = 20;
    const int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.clear();
        Engine::textMain.setCurrentColor(15);
    }
    heartSpr.setShown(true);
    heartSpr.wx = (optionX + offsetX) << 8;
    heartSpr.wy = (optionY + optionSpacingY * mercyFlee + offsetY) << 8;
    if (!draw)
        return;
    if (mercyText == nullptr)
        return;
    int x = optionX, y = optionY;
    for (char* p = mercyText; *p != 0; p++) {
        if (*p == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(fnt, *p, x, y);
    }
}

void BattleAction::drawTarget() {
    if (chosenTarget >= enemyCount)
        chosenTarget = enemyCount - 1;
    if (chosenTarget < 0)
        chosenTarget = 0;

    int enemyPageCount = enemyCount - (chosenTarget / 4) * 4;
    const int enemyNameX = 100, enemySpacing = 20;
    int enemyNameY = 96 - (enemySpacing * (enemyPageCount + 1) / 2);
    heartSpr.wx = (enemyNameX - 15) << 8;
    heartSpr.wy = (enemyNameY + enemySpacing * (chosenTarget % 4) + 4) << 8;

    if (chosenTarget / 4 == currentPage)
        return;
    currentPage = chosenTarget / 4;

    Engine::textMain.clear();
    for (int i = 0, enemyId = currentPage * 4; i < 4 && enemyId < enemyCount; i++, enemyId++) {
        // if (enemies[enemyId].spared || enemies[enemyId].hp <= 0)
        //     continue;
        int x = enemyNameX;
        int y = enemyNameY + i * enemySpacing;
        if (enemies[enemyId].spareValue >= 100)
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
                break;
            case ACTION_MERCY:
                enter(CHOOSING_MERCY);
                break;
        }
    }
    return false;
}

bool BattleAction::updateChoosingTarget() {
    if (keysDown() & KEY_DOWN)
        chosenTarget += 1;
    else if (keysDown() & KEY_UP)
        chosenTarget -= 1;
    else if (keysDown() & KEY_RIGHT)
        chosenTarget += 4;
    else if (keysDown() & KEY_LEFT)
        chosenTarget -= 4;
    drawTarget();
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
                return true;
            case ACTION_FIGHT:
                // return true;
                // TODO: Finish fight
                break;
            case ACTION_ACT:
                enter(CHOOSING_ACT);
                break;
        }
    }
    return false;
}

bool BattleAction::updateChoosingAct() {
    if (keysDown() & KEY_DOWN)
        chosenAct += 2;
    else if (keysDown() & KEY_UP)
        chosenAct -= 2;
    else if (keysDown() & KEY_RIGHT)
        chosenAct += 1;
    else if (keysDown() & KEY_LEFT)
        chosenAct -= 1;
    drawAct(false);
    if (keysDown() & KEY_B)
        enter(CHOOSING_TARGET);
    if (keysDown() & KEY_A)
        return true;
    return false;
}

bool BattleAction::updateChoosingMercy() {
    if (keysDown() & KEY_DOWN)
        mercyFlee = true;
    else if (keysDown() & KEY_UP)
        mercyFlee = false;
    drawMercy(false);
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    else if (keysDown() & KEY_A) {
        if (mercyFlee)
            return true;
        else
            enter(CHOOSING_TARGET);
    }
    return false;
}

bool BattleAction::updateChoosingItem() {
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    return false;
}

int BattleAction::getActionNum() const {
    switch (currentAction) {
        case ACTION_FIGHT:
            return 0 + chosenTarget;
        case ACTION_ACT:
            return 10 + chosenTarget * 4 + chosenAct;
        case ACTION_ITEM:
            return 60;
        case ACTION_MERCY:
            if (mercyFlee)
                return 40;
            return 41 + chosenTarget;
        default:
            nocashMessage("GetActionNum fail");
            return 0;
    }
}

void BattleAction::free_() {
    if (freed)
        return;

    globalSave.flags[230] = getActionNum();

    freed = true;
    delete[] mercyText;
    mercyText = nullptr;
    fightBtn.setShown(false);
    actBtn.setShown(false);
    itemBtn.setShown(false);
    mercyBtn.setShown(false);
    heartSpr.setShown(false);
    fightTex.free_();
    actTex.free_();
    itemTex.free_();
    mercyTex.free_();
    bigHeartTex.free_();
    smallHeartTex.free_();
    fnt.free_();
}
