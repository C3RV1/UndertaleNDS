//
// Created by cervi on 02/09/2022.
//

#include "Battle/Battle.hpp"
#include "Room/Room.hpp"
#include "Engine/Engine.hpp"
#include "Room/Player.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Formats/utils.hpp"

Battle* globalBattle = nullptr;

Battle::Battle() : _playerSpr(Engine::Allocated3D) {
    _playerTex.loadPath("spr_heartsmall");

    _playerSpr.loadTexture(_playerTex);
    _playerSpr._wx = ((256 - 16) / 2) << 8;
    _playerSpr._wy = ((192 - 32) / 2) << 8;
    _playerSpr._layer = 100;

    for (int i = 220; i <= 229; i++) {
        globalSave.flags[i] = 0;
    }

    FILE *f = fopen("nitro:/data/battle_win.txt", "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long len = ftell(f);
        fseek(f, 0, SEEK_SET);
        _winText = new char[len + 1];
        fread(_winText, len + 1, 1, f);
        _winText[len] = '\0';
    }
    fclose(f);
}

void Battle::exit(bool won) {
    if (won) {
        hide();
        int earnedExp = 0, earnedGold = 0;
        for (int i = 0; i < _enemyCount; i++) {
            if (_enemies[i]._hp <= 0)
                earnedExp += _enemies[i]._expOnKill;
            earnedGold += _enemies[i]._goldOnWin;
        }
        globalSave.exp += earnedExp;
        globalSave.gold += earnedGold;
        char buffer[200] = {0};
        sprintf(buffer, _winText, earnedExp, earnedGold);
        if (globalCutscene->_cDialogue == nullptr) {
            globalCutscene->_cDialogue = new Dialogue(true, 0, 0, buffer, "SND_TXT1.wav",
                                                      "fnt_maintext.font", 2,
                                                      Engine::textMain);
        }
        _stopPostDialogue = true;
    } else {
        _running = false;
    }
}

void Battle::loadFromStream(FILE *stream) {
    fread(&_enemyCount, 1, 1, stream);
    _enemies = new Enemy[_enemyCount];
    _cBattleAttacks = new BattleAttack*[_enemyCount];
    char buffer[100];
    for (int i = 0; i < _enemyCount; i++) {
        _cBattleAttacks[i] = nullptr;
        _enemies[i].readFromStream(stream);
    }

    u8 boardId;
    fread(&boardId, 1, 1, stream);
    sprintf(buffer, "battle/board%d", boardId);
    _bulletBoard.loadPath(buffer);

    fread(&_boardX, 1, 1, stream);
    fread(&_boardY, 1, 1, stream);
    fread(&_boardW, 1, 1, stream);
    fread(&_boardH, 1, 1, stream);

    _playerSpr._wx = ((_boardX + _boardW / 2) << 8) - (9 << 8) / 2;
    _playerSpr._wy = ((_boardY + _boardH / 2) << 8) - (9 << 8) / 2;
}

void Battle::show() {
    Engine::textMain.clear();
    _bulletBoard.loadBgTextMain();
    _playerSpr.setShown(true);
    _shown = true;
}

void Battle::hide() {
    Engine::textMain.clear();
    Engine::clearMain();
    _playerSpr.setShown(false);
    _shown = false;
}

void Battle::startBattleAttacks() {
    _hitFlag = false;
    for (int i = 0; i < _enemyCount; i++) {
        Enemy* enemy = &_enemies[i];
        if (!enemy->_spared && enemy->_hp > 0) {
            _cBattleAttacks[i] = getBattleAttack(enemy->_attackId);
        }
    }
}

void Battle::updateBattleAttacks() const {
    for (int i = 0; i < _enemyCount; i++) {
        BattleAttack* btlAttack = _cBattleAttacks[i];
        if (btlAttack != nullptr) {
            if (btlAttack->update()) {
                delete btlAttack;
                _cBattleAttacks[i] = nullptr;
            }
        }
    }
}

void Battle::update() {
    _nav.update();
    updateBattleAttacks();
    if (_cBattleAction != nullptr) {
        if (_cBattleAction->update()) {
            delete _cBattleAction;
            _cBattleAction = nullptr;
            show();
        } else {
            return;
        }
    }
    if (!_shown)
        return;
    if (keysHeld() & KEY_RIGHT) {
        _playerSpr._wx += _playerSpeed;
    }
    if (keysHeld() & KEY_LEFT) {
        _playerSpr._wx -= _playerSpeed;
    }
    if (keysHeld() & KEY_DOWN) {
        _playerSpr._wy += _playerSpeed;
    }
    if (keysHeld() & KEY_UP) {
        _playerSpr._wy -= _playerSpeed;
    }
    if (keysHeld() & KEY_TOUCH) {
        touchPosition touchInfo;
        touchRead(&touchInfo);
        _playerSpr._wx = (touchInfo.px << 8) - (9 << 8) / 2;
        _playerSpr._wy = (touchInfo.py << 8) - (9 << 8) / 2;
    }
    if (_playerSpr._wx < _boardX << 8) {
        _playerSpr._wx = _boardX << 8;
    } else if (_playerSpr._wx > (_boardX + _boardW - 9) << 8) {
        _playerSpr._wx = (_boardX + _boardW - 9) << 8;
    }
    if (_playerSpr._wy < _boardY << 8) {
        _playerSpr._wy = _boardY << 8;
    } else if (_playerSpr._wy > (_boardY + _boardH - 9) << 8) {
        _playerSpr._wy = (_boardY + _boardH - 9) << 8;
    }
}

void Battle::free_() {
    delete[] _winText;
    _winText = nullptr;
    _playerSpr.setShown(false);
    for (int i = 0; i < _enemyCount; i++) {
        delete[] _enemies[i]._actText;
        delete _cBattleAttacks[i];
    }
    delete[] _enemies;
    delete[] _cBattleAttacks;
    _enemies = nullptr;
    _enemyCount = 0;
    delete[] _textures;
    _textures = nullptr;
    for (int i = 0; i < _spriteCount; i++) {
        delete _sprites[i];
        _sprites[i] = nullptr;
    }
    delete[] _sprites;
    _sprites = nullptr;
}

void runBattle(FILE* stream) {
    int timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * (kRoomChangeFadeFrames - timer)) / kRoomChangeFadeFrames);
        timer--;
    }

    globalRoom->push();
    Engine::textMain.clear();
    Engine::textSub.clear();
    globalInGameMenu.unload();

    lcdMainOnBottom();

    globalBattle = new Battle();
    globalBattle->loadFromStream(stream);
    globalBattle->show();

    if (globalCutscene != nullptr) {
        globalCutscene->runCommands(LOAD_BATTLE);
    }

    timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * timer) / kRoomChangeFadeFrames);
        timer--;
    }

    while (globalBattle->_running) {
        Engine::tick();
        if (globalCutscene != nullptr) {
            if (globalBattle->_stopPostDialogue && globalCutscene->_cDialogue == nullptr) {
                globalBattle->_running = false;
            }
            globalCutscene->update();
            if (globalCutscene->runCommands(BATTLE)) {
                delete globalCutscene;
                globalCutscene = nullptr;
                globalInGameMenu.show(false);
                globalPlayer->setPlayerControl(true);
                globalCamera._manual = false;
            }
        }
        globalBattle->update();
    }

    timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * (kRoomChangeFadeFrames - timer)) / kRoomChangeFadeFrames);
        timer--;
    }

    delete globalBattle;
    Engine::textMain.clear();
    Engine::textSub.clear();

    lcdMainOnTop();

    globalRoom->pop();
    globalInGameMenu.load();

    if (globalCutscene != nullptr) {
        globalCutscene->runCommands(LOAD_ROOM);
    }
    globalCamera.updatePosition(true);
    globalPlayer->draw();
    globalRoom->draw();

    timer = kRoomChangeFadeFrames;
    while (timer >= 0) {
        Engine::tick();
        setBrightness(3, (-16 * timer) / kRoomChangeFadeFrames);
        timer--;
    }
}
