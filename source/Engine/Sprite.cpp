//
// Created by cervi on 30/08/2022.
//
#include "Engine/Sprite.hpp"

namespace Engine {
    Sprite::Sprite(Engine::AllocationMode allocMode) {
        _allocMode = allocMode;
    }

    void Sprite::setAllocationMode(Engine::AllocationMode allocMode) {
        if (allocMode == _allocMode)
            return;
        bool lastShown = _shown;
        setShown(false);
        _allocMode = allocMode;
        setShown(lastShown);
    }

    void Sprite::setAnimation(int animId) {
        if (!_loaded)
            return;
        if (animId >= _texture->_animationCount)
            return;
        if (_cAnimation == animId)
            return;
        _cAnimation = animId;
        _cAnimFrame = 0;
        _cAnimTimer = _texture->_animations[animId].frames[0].duration;
        _cFrame = _texture->_animations[animId].frames[0].frame;
    }

    void Sprite::loadTexture(Engine::Texture &texture) {
        if (!texture.getLoaded())
            return;
        if (&texture == _texture)
            return;
        push();

        _texture = &texture;
        _loaded = true;
        _cFrame = 0;
        _cAnimation = -1;

        int animId = nameToAnimId("gfx");  // default animation
        if (animId != -1)
            setAnimation(animId);
        pop();
    }

    void Sprite::tick() {
        if (!_loaded)
            return;
        if (_cAnimation >= 0) {
            CSPRAnimation *current = &_texture->_animations[_cAnimation];
            if (current->frames[_cAnimFrame].duration != 0) {
                _cAnimTimer--;
                if (_cAnimTimer == 0) {
                    _cAnimFrame++;
                    _cAnimFrame %= current->frames.size();
                    _cFrame = current->frames[_cAnimFrame].frame;
                    _cAnimTimer = current->frames[_cAnimFrame].duration;
                }
            }
        }

        _x = _wx - _cam_x;
        _y = _wy - _cam_y;
        if (_cAnimation >= 0) {
            CSPRAnimation* current = &_texture->_animations[_cAnimation];
            CSPRAnimFrame* frameInfo = &current->frames[_cAnimFrame];
            _x += frameInfo->drawOffX << 8;
            _y += frameInfo->drawOffY << 8;
        }
        _x *= _cam_scale_x;
        _x >>= 8;
        _y *= _cam_scale_y;
        _y >>= 8;
        _scale_x = (_cam_scale_x * _w_scale_x) >> 8;
        _scale_y = (_cam_scale_y * _w_scale_y) >> 8;
    }

    void Sprite::setFrame(int frameId) {
        if (!_loaded)
            return;
        _cAnimation = -1;
        _cFrame = frameId;
    }

    void Sprite::setShown(bool shown) {
        if (!_loaded)
            return;
        if (shown == _shown)
            return;
        _shown = shown;
        if (_shown) {
            if (_memory.allocated != NoAlloc)
                return;
            if (_allocMode == Allocated3D)
                main3dSpr.loadSprite(*this);
            else if (_allocMode == AllocatedOAM) {
                OAMManagerSub.loadSprite(*this);
            }
        } else {
            if (_memory.allocated == Allocated3D)
                main3dSpr.freeSprite(*this);
            else if (_memory.allocated == AllocatedOAM)
                OAMManagerSub.freeSprite(*this);
        }
    }

    int Sprite::nameToAnimId(const std::string &animName) const {
        if (!_loaded)
            return -1;
        for (int i = 0; i < _texture->_animationCount; i++) {
            if (animName == _texture->_animations[i].name) {
                return i;
            }
        }
        return -1;
    }

    void Sprite::push() {
        _pushed = _shown;
        setShown(false);
    }

    void Sprite::pop() {
        setShown(_pushed);
    }
}
