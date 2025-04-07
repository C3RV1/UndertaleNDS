//
// Created by cervi on 23/02/2024.
//

#ifndef UNDERTALE_FIGHTRESULT_HPP
#define UNDERTALE_FIGHTRESULT_HPP

#include <vector>
#include "Engine/Sprite.hpp"
#include "Engine/Engine.hpp"
#include "ManagedSprite.hpp"

class FightResult {
public:
    FightResult(Engine::Sprite* target, u8 offX, u8 offY);
    bool update();

private:
    Engine::Texture _numbersTex;
    std::vector<Engine::Sprite> _numbersSpr;
    Engine::Texture _sliceTex;
    Engine::Sprite _sliceSpr;
    Engine::Sprite *_target;

    u8 _offX, _offY;
};

#endif //UNDERTALE_FIGHTRESULT_HPP
