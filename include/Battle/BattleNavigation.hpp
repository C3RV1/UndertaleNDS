//
// Created by cervi on 23/06/2026
//

#ifndef UNDERTALE_BATTLE_NAVIGATION_HPP
#define UNDERTALE_BATTLE_NAVIGATION_HPP

#include "Cutscene/Navigation.hpp"

class Battle;

class BattleNavigation : public Navigation {
public:
  explicit BattleNavigation(Battle* battle);
  void spawn_sprite(const std::string &path, s32 x, s32 y, s32 layer) final;
  void unload_sprite(s8 sprId) final;
  std::shared_ptr<Engine::Sprite> getTarget(const TargetInfo &targetInfo) final;

private:
  Battle* _battle;
};

#endif
