//
// Created by cervi on 28/08/2022.
//

#include "ManagedSprite.hpp"
#include "Room/Camera.hpp"
#include "Room/Player.hpp"
#include "Engine/math.hpp"

void ManagedSprite::load(ROOMSprite const& sprData,
                         std::vector<std::shared_ptr<Engine::Texture>>& textures) {
    if (sprData.textureId < textures.size()) {
        _texture = textures[sprData.textureId];
        _spr.loadTexture(*_texture);
    }
    _animationId = _spr.nameToAnimId(sprData.animation);
    _spr._wx = sprData.x << 8;
    _spr._wy = sprData.y << 8;
    _spr.setAnimation(_animationId);

    _spr.setShown(true);

    _interactAction = sprData.interactAction;
    _parallax_x = 1 << 8;
    _parallax_y = 1 << 8;
    if (_interactAction == 1)
        _cutsceneId = sprData.cutsceneId;
    else if (_interactAction == 2) {
        _distanceSquared = sprData.distance * sprData.distance;
        _closeAnim = _spr.nameToAnimId(sprData.closeAnim);
    }
    else if (_interactAction == 3) {
        _parallax_x = sprData.parallax_x;
        _parallax_y = sprData.parallax_y;
    }
}

void ManagedSprite::spawn(s8 textureId, s32 x, s32 y,
                          std::vector<std::shared_ptr<Engine::Texture>>& textures) {
    u8 texId2;
    if (textureId < 0)
        texId2 = textures.size() + textureId;
    else
        texId2 = textureId;
    if (texId2 < textures.size()) {
        _texture = textures[texId2];
        _spr.loadTexture(*_texture);
    }
    _spr._wx = x;
    _spr._wy = y;

    _spr.setShown(true);
}

void ManagedSprite::draw(bool isRoom) {
    if (isRoom) {
        _spr._cam_x = (globalCamera._pos._wx * _parallax_x) >> 8;
        _spr._cam_y = (globalCamera._pos._wy * _parallax_y) >> 8;
        _spr._cam_scale_x = globalCamera._pos._w_scale_x;
        _spr._cam_scale_y = globalCamera._pos._w_scale_y;
        _spr._layer = _spr._wy >> 8;
    }
}

void ManagedSprite::update(bool isRoom) {
    if (isRoom && _interactAction == 2) {
        if (_spr._texture == nullptr)
            return;
        if (globalPlayer->_playerSpr._texture == nullptr)
            return;
        u16 width = _spr._texture->getWidth();
        u16 height = _spr._texture->getHeight();
        u16 pw = globalPlayer->_playerSpr._texture->getWidth();
        u16 ph = globalPlayer->_playerSpr._texture->getHeight();
        u32 distance = distSquared_fp(_spr._wx + width / 2,
                                      _spr._wy + height / 2,
                                      globalPlayer->_playerSpr._wx + pw / 2,
                                      globalPlayer->_playerSpr._wy + ph / 2);
        if (distance >> 8 < _distanceSquared)
            _spr.setAnimation(_closeAnim);
        else
            _spr.setAnimation(_animationId);
    }
}

void ManagedSprite::free_() {
    _spr.setShown(false);
}