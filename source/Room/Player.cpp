//
// Created by cervi on 27/08/2022.
//

#include "Room/Player.hpp"
#include "Room/Room.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Room/Camera.hpp"
#include "Engine/math.hpp"

Player::Player() : _playerSpr(Engine::Allocated3D) {
    _playerTex.loadPath("mainchara");
    _playerSpr.loadTexture(_playerTex);

    _upIdleId = _playerSpr.nameToAnimId("upIdle");
    _downIdleId = _playerSpr.nameToAnimId("downIdle");
    _leftIdleId = _playerSpr.nameToAnimId("leftIdle");
    _rightIdleId = _playerSpr.nameToAnimId("rightIdle");
    _upMoveId = _playerSpr.nameToAnimId("upMove");
    _downMoveId = _playerSpr.nameToAnimId("downMove");
    _leftMoveId = _playerSpr.nameToAnimId("leftMove");
    _rightMoveId = _playerSpr.nameToAnimId("rightMove");
}

void Player::setPlayerControl(bool playerControl) {
    _playerControl = playerControl;
    if (!_playerControl) {
        if (_playerSpr._cAnimation == _upMoveId)
            _playerSpr.setAnimation(_upIdleId);
        else if (_playerSpr._cAnimation == _downMoveId)
            _playerSpr.setAnimation(_downIdleId);
        else if (_playerSpr._cAnimation == _leftMoveId)
            _playerSpr.setAnimation(_leftIdleId);
        else if (_playerSpr._cAnimation == _rightMoveId)
            _playerSpr.setAnimation(_rightIdleId);
    }
}

void Player::update() {
    if (!_playerControl)
        return;
    s32 prevX = _playerSpr._wx, prevY = _playerSpr._wy;
    int moveDirection = -1;
    bool setAnim = true;
    if (keysHeld() & KEY_DOWN) {
        _playerSpr._wy += kMoveSpeed;
        moveDirection = _downMoveId;
        if (_playerSpr._cAnimation == _downMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_UP) {
        _playerSpr._wy -= kMoveSpeed;
        moveDirection = _upMoveId;
        if (_playerSpr._cAnimation == _upMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_RIGHT) {
        _playerSpr._wx += kMoveSpeed;
        moveDirection = _rightMoveId;
        if (_playerSpr._cAnimation == _rightMoveId)
            setAnim = false;
    }
    if (keysHeld() & KEY_LEFT) {
        _playerSpr._wx -= kMoveSpeed;
        moveDirection = _leftMoveId;
        if (_playerSpr._cAnimation == _leftMoveId)
            setAnim = false;
    }

    if (prevX != _playerSpr._wx || prevY != _playerSpr._wy) {
        check_exits();
        int x2 = _playerSpr._wx, y2 = _playerSpr._wy;
        if (check_collisions()) {
            _playerSpr._wx = prevX;
            _playerSpr._wy = y2;
            if (check_collisions()) {
                _playerSpr._wx = x2;
                _playerSpr._wy = prevY;
                if (check_collisions()) {
                    _playerSpr._wx = prevX;
                    _playerSpr._wy = prevY;
                }
            }
        }
    }

    if (_playerSpr._wx == prevX && prevY == _playerSpr._wy) {
        if (moveDirection == -1)
            moveDirection = _playerSpr._cAnimation;
        if (moveDirection == _upMoveId)
            _playerSpr.setAnimation(_upIdleId);
        else if (moveDirection == _downMoveId)
            _playerSpr.setAnimation(_downIdleId);
        else if (moveDirection == _leftMoveId)
            _playerSpr.setAnimation(_leftIdleId);
        else if (moveDirection == _rightMoveId)
            _playerSpr.setAnimation(_rightIdleId);
    } else {
        if (setAnim) {
            _playerSpr.setAnimation(moveDirection);
        }
    }

    if (keysDown() & KEY_A) {
        check_interact();
    }
}

void Player::check_exits() {
    u16 width, height;
    globalRoom->_bg.getSize(width, height);
    if (_playerSpr._wx < 0) {
        _playerSpr._wx = 0;
        if (globalRoom->_exitLeft != nullptr) {
            loadNewRoom(globalRoom->_exitLeft->roomId,
                        globalRoom->_exitLeft->spawnX,
                        globalRoom->_exitLeft->spawnY);
        }
    }
    else if ((_playerSpr._wx >> 8) + 19 > width * 8) {
        _playerSpr._wx = (width * 8 - 19) << 8;
        if (globalRoom->_exitRight != nullptr) {
            loadNewRoom(globalRoom->_exitRight->roomId,
                        globalRoom->_exitRight->spawnX,
                        globalRoom->_exitRight->spawnY);
        }
    }
    if (_playerSpr._wy < 0) {
        _playerSpr._wy = 0;
        if (globalRoom->_exitTop != nullptr) {
            loadNewRoom(globalRoom->_exitTop->roomId,
                        globalRoom->_exitTop->spawnX,
                        globalRoom->_exitTop->spawnY);
        }
    }
    else if ((_playerSpr._wy >> 8) + 29 > height * 8) {
        _playerSpr._wy = (height * 8 - 29) << 8;
        if (globalRoom->_exitBtm != nullptr) {
            loadNewRoom(globalRoom->_exitBtm->roomId,
                        globalRoom->_exitBtm->spawnX,
                        globalRoom->_exitBtm->spawnY);
        }
    }

    for (int i = 0; i < globalRoom->_rectExitCount; i++) {
        ROOMExit* rectExit = globalRoom->_rectExits[i];
        if (collidesRect(_playerSpr._wx >> 8, (_playerSpr._wy >> 8) + 20, 19, 9,
                         rectExit->x, rectExit->y,
                         rectExit->w, rectExit->h)) {
            loadNewRoom(rectExit->roomId,
                        rectExit->spawnX,
                        rectExit->spawnY);
        }
    }
}

void Player::check_interact() const {
    s32 x, y, w = 19, h = 9, x2, y2, w2, h2;
    if (_playerSpr._cAnimation == _upIdleId || _playerSpr._cAnimation == _upMoveId) {
        x = 0;
        y = -9;
        h = 19;
    } else if (_playerSpr._cAnimation == _downIdleId || _playerSpr._cAnimation == _downMoveId) {
        x = 0;
        y = 29;
    } else if (_playerSpr._cAnimation == _rightIdleId || _playerSpr._cAnimation == _rightMoveId) {
        x = 19;
        y = 29 - 9;
    } else {
        x = -19;
        y = 29 - 9;
    }
    x += _playerSpr._wx >> 8;
    y += _playerSpr._wy >> 8;
    for (auto & sprite : globalRoom->_sprites) {
        if (sprite->_interactAction == 0)
            continue;
        x2 = sprite->_spr._wx >> 8;
        y2 = sprite->_spr._wy >> 8;
        if (sprite->_spr._texture == nullptr)
            continue;
        w2 = sprite->_spr._texture->getWidth();
        h2 = sprite->_spr._texture->getHeight();
        if (collidesRect(x, y, w, h, x2, y2, w2, h2)) {
            if (sprite->_interactAction == 1) {
                if (globalCutscene == nullptr)
                    globalCutscene = std::make_unique<Cutscene>(
                            sprite->_cutsceneId,
                            globalRoom->_roomId);
                return;
            }
        }
    }
}

bool Player::check_collisions() const {
    for (auto & collider : globalRoom->_roomData.roomColliders.roomColliders) {
        if (!collider.enabled)
            continue;
        if (collidesRect(_playerSpr._wx >> 8, (_playerSpr._wy >> 8) + 20, 19, 9,
                         collider.x, collider.y,
                         collider.w, collider.h)) {
            if (collider.colliderAction == 0)  // Wall
                return true;
            if (collider.colliderAction == 1 && globalCutscene == nullptr) {  // Cutscene
                globalCutscene = std::make_unique<Cutscene>(
                        collider.cutsceneId,
                        globalRoom->_roomId);
            }
        }
    }
    return false;
}

void Player::draw() {
    _playerSpr._cam_x = globalCamera._pos._wx;
    _playerSpr._cam_y = globalCamera._pos._wy;
    _playerSpr._cam_scale_x = globalCamera._pos._w_scale_x;
    _playerSpr._cam_scale_y = globalCamera._pos._w_scale_y;
    _playerSpr._layer = _playerSpr._wy >> 8;
}

Player* globalPlayer;
