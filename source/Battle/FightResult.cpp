//
// Created by cervi on 23/02/2024.
//
#ifdef DISABLE
#include "Battle/FightResult.hpp"
#include "Save.hpp"

FightResult::FightResult(Engine::Sprite *target, u8 offX, u8 offY)
    : _sliceSpr(Engine::Allocated3D){
    _target = target;
    _offX = offX;
    _offY = offY;
    if (_target == nullptr) {
        Engine::throw_("Trying to show a fight result and target is null.");
    }

    _sliceTex.loadPath("battle/spr_slice_o");
    _sliceSpr.loadTexture(_sliceTex);
    _sliceSpr._wx = _target->_wx + (_offX << 8);
    _sliceSpr._wy = _target->_wy + (_offY << 8);

    _numbersTex.loadPath("battle/damage_numbers");

    u16 damage = globalSave.flags[FlagIds::LAST_DAMAGE];
    do {
        Engine::Sprite damageNumber(Engine::Allocated3D);
        damageNumber.loadTexture(_numbersTex);
        damageNumber.setShown(true);
        damageNumber.setAnimation(damage % 10);
        _numbersSpr.push_back(std::move(damageNumber));
        damage /= 10;
    } while (damage > 0);

    s32 posX = (_sliceSpr._wx + (_numbersTex.getWidth() << 8) * (s32)_numbersSpr.size() / 2);
    s32 posY = (_sliceSpr._wy - (33 << 8));
    for (auto & spr : _numbersSpr) {
        spr._wx = posX;
        spr._wy = posY;
        posX -= _numbersTex.getWidth() << 8;
    }
}

bool FightResult::update() {
    return _sliceSpr.getFrame() >= 5;
}
#endif
