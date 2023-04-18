//
// Created by cervi on 02/10/2022.
//

#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"

// TODO: Touchscreen

BattleAction::BattleAction(u8 enemyCount_, Enemy* enemies) :
        _enemyCount(enemyCount_), _enemies(enemies),
        _fightBtn(Engine::Allocated3D),
        _actBtn(Engine::Allocated3D),
        _itemBtn(Engine::Allocated3D),
        _mercyBtn(Engine::Allocated3D),
        _heartSpr(Engine::Allocated3D),
        _attackSpr(Engine::Allocated3D)
{
    _fnt.loadPath("fnt_maintext.font");

    _fightBoard.loadPath("fight_board");
    _damageNumbers.loadPath("battle/damage_numbers");
    _missText.loadPath("battle/miss_text");
    _attackTex.loadPath("battle/spr_targetchoice");
    _attackSpr.loadTexture(_attackTex);

    _fightTex.loadPath("btn/spr_fightbt");
    _fightBtn.loadTexture(_fightTex);
    _fightBtn._wx = 12 << 8; _fightBtn._wy = 36 << 8;

    _actTex.loadPath("btn/spr_talkbt");
    _actBtn.loadTexture(_actTex);
    _actBtn._wx = 134 << 8; _actBtn._wy = 36 << 8;

    _itemTex.loadPath("btn/spr_itembt");
    _itemBtn.loadTexture(_itemTex);
    _itemBtn._wx = 12 << 8; _itemBtn._wy = 114 << 8;

    _mercyTex.loadPath("btn/spr_sparebt");
    _mercyBtn.loadTexture(_mercyTex);
    _mercyBtn._wx = 134 << 8; _mercyBtn._wy = 114 << 8;

    _gfxAnimId = _fightBtn.nameToAnimId("gfx");
    _activeAnimId = _fightBtn.nameToAnimId("active");

    _bigHeartTex.loadPath("spr_heart");
    _smallHeartTex.loadPath("spr_heartsmall");
    _heartSpr._layer = 3;

    FILE *f = fopen("nitro:/data/mercy.txt", "rb");
    if (f) {
        int len = str_len_file(f, '@');
        _mercyText = new char[len + 1];
        fread(_mercyText, len + 1, 1, f);
        _mercyText[len] = '\0';
    } else {
        nocashMessage("Error opening mercy text");
    }
    fclose(f);

    enter(CHOOSING_ACTION);
}

void BattleAction::enter(BattleActionState state) {
    _cState = state;
    _fightBtn.setShown(state == CHOOSING_ACTION);
    _actBtn.setShown(state == CHOOSING_ACTION);
    _itemBtn.setShown(state == CHOOSING_ACTION);
    _mercyBtn.setShown(state == CHOOSING_ACTION);
    switch (state) {
        case CHOOSING_ACTION:
            _cAction = ACTION_FIGHT;
            Engine::textMain.clear();
            _heartSpr.loadTexture(_bigHeartTex);
            _heartSpr.setShown(true);
            setBtn();
            break;
        case CHOOSING_TARGET:
            _cTarget = 0;
            _cPage = -1;
            _heartSpr.loadTexture(_smallHeartTex);
            _heartSpr.setShown(true);
            drawTarget();
            break;
        case CHOOSING_ACT:
            _cAct = 0;
            _heartSpr.loadTexture(_smallHeartTex);
            _heartSpr.setShown(true);
            drawAct(true);
            break;
        case CHOOSING_MERCY:
            _mercyFlee = false;
            _heartSpr.loadTexture(_smallHeartTex);
            _heartSpr.setShown(true);
            drawMercy(true);
            break;
        case FIGHTING:
            _fightBoard.loadBgTextMain();
            _attackSpr.setShown(true);
            _attackSpr._wx = 0;
            _attackSpr._wy = (192 - _attackTex.getHeight()) / 2;
            Engine::textMain.clear();
            break;
        case CHOOSING_ITEM:
            break;
    }
}

void BattleAction::drawAct(bool draw) {
    const int optionX = 40, optionY = 50, optionSpacingX = 90, optionSpacingY = 20;
    const int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.clear();
        Engine::textMain.setColor(15);
    }
    if (_cAct < 0)
        _cAct = 0;
    if (_cAct >= _enemies[_cTarget]._actOptionCount)
        _cAct = _enemies[_cTarget]._actOptionCount - 1;
    if (_cAct < 0) {
        _heartSpr.setShown(false);
        return;
    }
    _heartSpr.setShown(true);
    _heartSpr._wx = (optionX + optionSpacingX * (_cAct % 2) + offsetX) << 8;
    _heartSpr._wy = (optionY + optionSpacingY * (_cAct / 2) + offsetY) << 8;
    if (!draw)
        return;
    if (_enemies[_cTarget]._actText == nullptr)
        return;
    int x = optionX, y = optionY;
    for (char* p = _enemies[_cTarget]._actText; *p != 0; p++) {
        if (*p == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(_fnt, *p, x, y);
    }
}

void BattleAction::drawMercy(bool draw) {
    const int optionX = 100, optionY = 66, optionSpacingY = 20;
    const int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.clear();
        Engine::textMain.setColor(15);
    }
    _heartSpr.setShown(true);
    _heartSpr._wx = (optionX + offsetX) << 8;
    _heartSpr._wy = (optionY + optionSpacingY * _mercyFlee + offsetY) << 8;
    if (!draw)
        return;
    if (_mercyText == nullptr)
        return;
    int x = optionX, y = optionY;
    for (char* p = _mercyText; *p != 0; p++) {
        if (*p == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(_fnt, *p, x, y);
    }
}

void BattleAction::drawTarget() {
    // TODO: Do not show nor allow spared or killed enemies
    if (_cTarget >= _enemyCount)
        _cTarget = _enemyCount - 1;
    if (_cTarget < 0)
        _cTarget = 0;

    int enemyPageCount = _enemyCount - (_cTarget / 4) * 4;
    const int enemyNameX = 100, enemySpacing = 20;
    int enemyNameY = 96 - (enemySpacing * (enemyPageCount + 1) / 2);
    _heartSpr._wx = (enemyNameX - 15) << 8;
    _heartSpr._wy = (enemyNameY + enemySpacing * (_cTarget % 4) + 4) << 8;

    if (_cTarget / 4 == _cPage)
        return;
    _cPage = _cTarget / 4;

    Engine::textMain.clear();
    for (int i = 0, enemyId = _cPage * 4; i < 4 && enemyId < _enemyCount; i++, enemyId++) {
        // if (enemies[enemyId].spared || enemies[enemyId].hp <= 0)
        //     continue;
        int x = enemyNameX;
        int y = enemyNameY + i * enemySpacing;
        if (_enemies[enemyId]._spareValue >= 100)
            Engine::textMain.setColor(12);
        else
            Engine::textMain.setColor(15);
        Engine::textMain.drawGlyph(_fnt, '*', x, y);
        Engine::textMain.drawGlyph(_fnt, ' ', x, y);
        for (char *p = _enemies[enemyId]._enemyName; *p != 0; p++) {
            Engine::textMain.drawGlyph(_fnt, *p, x, y);
        }
    }
}

void BattleAction::setBtn() {
    Engine::Sprite* activeSpr = nullptr;
    _fightBtn.setSpriteAnim(_gfxAnimId);
    _actBtn.setSpriteAnim(_gfxAnimId);
    _itemBtn.setSpriteAnim(_gfxAnimId);
    _mercyBtn.setSpriteAnim(_gfxAnimId);

    switch (_cAction) {
        case ACTION_FIGHT:
            activeSpr = &_fightBtn;
            break;
        case ACTION_ACT:
            activeSpr = &_actBtn;
            break;
        case ACTION_ITEM:
            activeSpr = &_itemBtn;
            break;
        case ACTION_MERCY:
            activeSpr = &_mercyBtn;
            break;
    }

    if (activeSpr != nullptr) {
        activeSpr->setSpriteAnim(_activeAnimId);
        _heartSpr._wx = activeSpr->_wx + (8 << 8);
        _heartSpr._wy = activeSpr->_wy + (13 << 8);
    }
}

bool BattleAction::update() {
    switch (_cState) {
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
        case FIGHTING:
            return updateFighting();
    }
    return true;
}

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
        switch (_cAction) {
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
        _cTarget += 1;
    else if (keysDown() & KEY_UP)
        _cTarget -= 1;
    else if (keysDown() & KEY_RIGHT)
        _cTarget += 4;
    else if (keysDown() & KEY_LEFT)
        _cTarget -= 4;
    drawTarget();
    if (keysDown() & KEY_B) {
        switch (_cAction) {
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
        switch (_cAction) {
            case ACTION_MERCY:
                return true;
            case ACTION_FIGHT:
                enter(FIGHTING);
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
        _cAct += 2;
    else if (keysDown() & KEY_UP)
        _cAct -= 2;
    else if (keysDown() & KEY_RIGHT)
        _cAct += 1;
    else if (keysDown() & KEY_LEFT)
        _cAct -= 1;
    drawAct(false);
    if (keysDown() & KEY_B)
        enter(CHOOSING_TARGET);
    if (keysDown() & KEY_A)
        return true;
    return false;
}

bool BattleAction::updateChoosingMercy() {
    if (keysDown() & KEY_DOWN)
        _mercyFlee = true;
    else if (keysDown() & KEY_UP)
        _mercyFlee = false;
    drawMercy(false);
    if (keysDown() & KEY_B)
        enter(CHOOSING_ACTION);
    else if (keysDown() & KEY_A) {
        if (_mercyFlee)
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

bool BattleAction::updateFighting() {
    // TODO: Show damage

    if (keysDown() & (KEY_A | KEY_TOUCH)) {
        s32 distanceFromCenter = _attackSpr._wx - (128 << 8);
        if (distanceFromCenter < 0)
            distanceFromCenter = -distanceFromCenter;
        s16 damage;
        u8 weaponAtk = 0; // TODO: get attack from current weapon
        u8 atk = 10 + weaponAtk;
        Enemy* enemy = &_enemies[_cTarget];
        double randValue = ((double)rand() / (double)RAND_MAX) * 2.0;
        if (distanceFromCenter < 24 << 8) {
            // round((atk - def + rand(2)) * 2.2)
            damage =  (s16)((atk - enemy->_defense + randValue) * 22.0 / 10.0);
        } else {
            // round((atk - def + rand(2)) * (1 - distance from center/target width) * 2)
            double distanceFactor = 1.0 - ((double)(distanceFromCenter >> 8) - 24) / (128.0 - 24.0);
            damage = (s16)((atk - enemy->_defense + randValue) * distanceFactor * 2.0);
        }
        if (damage < 0)
            damage = 0;
        char buffer[200];
        sprintf(buffer, "Damage %d\n", damage);
        nocashMessage(buffer);
        enemy->_hp -= damage;
        if (enemy->_hp < 0)
            enemy->_hp = 0;
        return true;
    }
    if (_attackSpr._wx > 256 << 8) {
        return true;
    }
    _attackSpr._wx += kAttackSpeed;
    return false;
}

int BattleAction::getActionNum() const {
    switch (_cAction) {
        case ACTION_FIGHT:
            return 0 + _cTarget;
        case ACTION_ACT:
            return 10 + _cTarget * 4 + _cAct;
        case ACTION_ITEM:
            return 60;
        case ACTION_MERCY:
            if (_mercyFlee)
                return 40;
            return 41 + _cTarget;
        default:
            nocashMessage("GetActionNum fail");
            return 0;
    }
}

void BattleAction::free_() {
    if (_freed)
        return;

    globalSave.flags[230] = getActionNum();

    _freed = true;
    delete[] _mercyText;
    _mercyText = nullptr;
    _fightBtn.setShown(false);
    _actBtn.setShown(false);
    _itemBtn.setShown(false);
    _mercyBtn.setShown(false);
    _heartSpr.setShown(false);
    _fightTex.free_();
    _actTex.free_();
    _itemTex.free_();
    _mercyTex.free_();
    _bigHeartTex.free_();
    _smallHeartTex.free_();
    _fnt.free_();
}
