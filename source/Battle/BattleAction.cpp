//
// Created by cervi on 02/10/2022.
//

#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"

#include "Formats/utils.hpp"
#include "Save.hpp"

// TODO: Touchscreen

BattleAction::BattleAction(std::vector<Enemy>* enemies,
                           int flavorTextId) :
        _enemies(enemies),
        _bigHeartSpr(Engine::Allocated3D),
        _smallHeartSpr(Engine::Allocated3D),
        _attackSpr(Engine::Allocated3D)
{
    _fnt.loadPath("fnt_maintext.font");
    _selectSnd.loadWAV("snd_select.wav");

    _fightBoard.loadPath("fight_board");
    _damageNumbers.loadPath("battle/damage_numbers");
    _missText.loadPath("battle/miss_text");
    _attackTex.loadPath("battle/spr_targetchoice");
    _attackSpr.loadTexture(_attackTex);

    _textures[ACTION_FIGHT].loadPath("btn/spr_fightbt");
    _textures[ACTION_ACT].loadPath("btn/spr_talkbt");
    _textures[ACTION_ITEM].loadPath("btn/spr_itembt");
    _textures[ACTION_MERCY].loadPath("btn/spr_sparebt");

    for (int i = 0; i < 4; i++) {
        _btn[i].setAllocationMode(Engine::Allocated3D);
        _btn[i].loadTexture(_textures[i]);
        _btn[i]._wx = _buttonPositions[i][0];
        _btn[i]._wy = _buttonPositions[i][1];
        _btn[i].setShown(true);
    }

    _gfxAnimId = _btn[0].nameToAnimId("gfx");
    _activeAnimId = _btn[0].nameToAnimId("active");

    _bigHeartTex.loadPath("spr_heart");
    _smallHeartTex.loadPath("spr_heartsmall");
    _bigHeartSpr.loadTexture(_bigHeartTex);
    _bigHeartSpr.setShown(false);
    _smallHeartSpr.loadTexture(_smallHeartTex);
    _bigHeartSpr._layer = 3;
    _smallHeartSpr._layer = 3;

    FILE *f = fopen("nitro:/data/mercy.txt", "rb");
    if (!f)
        Engine::throw_("Error opening mercy text");
    int len = str_len_file(f, '@');
    _mercyText.resize(len);
    fread(&_mercyText[0], len, 1, f);
    fseek(f, 1, SEEK_CUR);
    fclose(f);

    if (flavorTextId == -1) {
        enter(CHOOSING_ACTION);
        return;
    }
    std::string buffer = "nitro:/data/flavorTexts/" + std::to_string(flavorTextId) + ".txt";
    f = fopen(buffer.c_str(), "rb");
    if (!f) {
        Engine::throw_("Error opening flavor text " + std::to_string(flavorTextId));
    }
    len = str_len_file(f, '\0');
    _flavorText.resize(len);
    fread(&_flavorText[0], len, 1, f);
    // fseek(f, 1, SEEK_CUR); FIXME: not needed right?
    fclose(f);

    enter(PRINTING_FLAVOR_TEXT);
}

void BattleAction::enter(BattleActionState state) {
    _cState = state;
    _bigHeartSpr.setShown(
            state == CHOOSING_ACTION
            );
    _smallHeartSpr.setShown(
            state != PRINTING_FLAVOR_TEXT &&
            state != CHOOSING_ACTION &&
            state != FIGHTING
            );
    _flavorTextDialogue->setShown(
            state != FIGHTING
            );
    Engine::textMain.clear();
    if (state != PRINTING_FLAVOR_TEXT and state != FIGHTING) {
        if (_flavorTextDialogue)
            _flavorTextDialogue->doRedraw();
    }
    switch (state) {
        case PRINTING_FLAVOR_TEXT:
            _flavorTextDialogue = std::make_unique<FlavorTextDialogue>(_flavorText);
            _flavorTextDialogue->setShown(true);
            break;
        case CHOOSING_ACTION:
            setBtn();
            break;
        case CHOOSING_TARGET:
            _cTarget = 0;
            _cPage = -1;
            drawTarget();
            break;
        case CHOOSING_ACT:
            _cAct = 0;
            drawAct(true);
            break;
        case CHOOSING_MERCY:
            _mercyFlee = false;
            drawMercy(true);
            break;
        case FIGHTING:
            _fightBoard.loadBgTextMain();
            _attackSpr.setShown(true);
            _attackSpr._wx = 0;
            _attackSpr._wy = (192 - _attackTex.getHeight()) / 2;
            break;
        case CHOOSING_ITEM:
            _smallHeartSpr.setShown(false);
            break;
    }
}

void BattleAction::drawAct(bool draw) {
    constexpr int optionX = 50, optionY = 110, optionSpacingX = 90, optionSpacingY = 20;
    constexpr int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.setColor(15);
    }
    if (_cAct < 0)
        _cAct = 0;
    if (_cAct >= (*_enemies)[_cTarget]._actOptionCount)
        _cAct = (*_enemies)[_cTarget]._actOptionCount - 1;
    if (_cAct < 0) {
        _smallHeartSpr.setShown(false);
        return;
    }
    _smallHeartSpr.setShown(true);
    _smallHeartSpr._wx = (optionX + optionSpacingX * (_cAct % 2) + offsetX) << 8;
    _smallHeartSpr._wy = (optionY + optionSpacingY * (_cAct / 2) + offsetY) << 8;
    if (!draw)
        return;
    int x = optionX, y = optionY;
    for (auto const & c : (*_enemies)[_cTarget]._actText) {
        if (c == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(_fnt, c, x, y);
    }
}

void BattleAction::drawMercy(bool draw) {
    const int optionX = 100, optionY = 110, optionSpacingY = 20;
    const int offsetX = -15, offsetY = 4;
    if (draw) {
        Engine::textMain.setColor(15);
    }
    _smallHeartSpr.setShown(true);
    _smallHeartSpr._wx = (optionX + offsetX) << 8;
    _smallHeartSpr._wy = (optionY + optionSpacingY * _mercyFlee + offsetY) << 8;
    if (!draw)
        return;
    int x = optionX, y = optionY;
    for (auto const & c : _mercyText) {
        if (c == '\n') {
            x = optionX;
            y += optionSpacingY;
            continue;
        }
        Engine::textMain.drawGlyph(_fnt, c, x, y);
    }
}

void BattleAction::drawTarget() {
    // TODO: Do not show allow spared nor killed enemies
    if (_cTarget >= (*_enemies).size())
        _cTarget = (*_enemies).size() - 1;
    if (_cTarget < 0)
        _cTarget = 0;

    int enemyPageCount = (*_enemies).size() - (_cTarget / 4) * 4;
    const int enemyNameX = 100, enemySpacing = 20;
    int enemyNameY = 120 - (enemySpacing * (enemyPageCount + 1) / 2);
    _smallHeartSpr._wx = (enemyNameX - 15) << 8;
    _smallHeartSpr._wy = (enemyNameY + enemySpacing * (_cTarget % 4) + 4) << 8;

    if (_cTarget / 4 == _cPage)
        return;
    _cPage = _cTarget / 4;

    for (int i = 0, enemyId = _cPage * 4; i < 4 && enemyId < (*_enemies).size(); i++, enemyId++) {
        // if (enemies[enemyId].spared || enemies[enemyId].hp <= 0)
        //     continue;
        int x = enemyNameX;
        int y = enemyNameY + i * enemySpacing;
        if ((*_enemies)[enemyId]._spareValue >= 100)
            Engine::textMain.setColor(12);
        else
            Engine::textMain.setColor(15);
        Engine::textMain.drawGlyph(_fnt, '*', x, y);
        Engine::textMain.drawGlyph(_fnt, ' ', x, y);
        for (auto const & c : (*_enemies)[enemyId]._enemyName) {
            Engine::textMain.drawGlyph(_fnt, c, x, y);
        }
    }
}

void BattleAction::setBtn() {
    for (auto & btn : _btn) {
        btn.setShown(true);
        btn.setAnimation(_gfxAnimId);
    }

    _btn[_cAction].setAnimation(_activeAnimId);
    _bigHeartSpr._wx = _btn[_cAction]._wx + (8 << 8);
    _bigHeartSpr._wy = _btn[_cAction]._wy + (13 << 8);
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
        case PRINTING_FLAVOR_TEXT:
            return updatePrintingFlavor();
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
    if (keysDown() & KEY_B) {
        enter(CHOOSING_ACTION);
    }
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
        Enemy* enemy = &(*_enemies)[_cTarget];
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

BattleAction::~BattleAction() {
    globalSave.flags[FlagIds::BATTLE_ACTION] = getActionNum();
}

bool BattleAction::updatePrintingFlavor() {
    if (_flavorTextDialogue->update())
        enter(CHOOSING_ACTION);
    return false;
}
