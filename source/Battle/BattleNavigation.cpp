#include "Battle/BattleNavigation.hpp"
#include "Battle/Battle.hpp"
#include "Engine/Engine.hpp"
#include <memory>
#include <string>
#include <utility>

BattleNavigation::BattleNavigation(Battle* battle): _battle(battle) {}

void BattleNavigation::spawn_sprite(u16 sprId, const std::string &path, s32 x,
                                    s32 y, s32 layer) {
  auto newSprite = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  newSprite->_wx = x;
  newSprite->_wy = y;
  newSprite->_layer = layer;
  Engine::spriteLoadTexture(newSprite, path);
  Engine::spriteSetShown(newSprite, true);

  if (sprId != 0 && _battle->_sprites.count(sprId) > 0)
    Engine::throw_("BattleNav: Duplicate spr id != 0: " +
                   std::to_string(sprId));
  _battle->_sprites.emplace(sprId, newSprite);
}

void BattleNavigation::unload_sprite(u16 sprId) {
  if (sprId == 0)
    return;
  _battle->_sprites.erase(sprId);
}

std::shared_ptr<Engine::Sprite>
BattleNavigation::getTarget(const TargetInfo &targetInfo) {
  TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
  switch (targetType) {
  case TargetType::NULL_:
    return nullptr;
  case TargetType::SPRITE: {
    auto it = _battle->_sprites.find(targetInfo.targetId);
    if (it != _battle->_sprites.end())
      return it->second;
    Engine::throw_("BattleNav: Target sprite with id " +
                   std::to_string(targetInfo.targetId) + " not found.");
    return nullptr;
  }
  case TargetType::ENEMY: {
    u8 enemyTargetId2;
    if (targetInfo.enemyTargetId < 0)
      enemyTargetId2 = _battle->_enemies.size() + targetInfo.enemyTargetId;
    else
      enemyTargetId2 = targetInfo.enemyTargetId;
    if (enemyTargetId2 >= _battle->_enemies.size()) {
      Engine::throw_("Error: target id outside of enemy count");
      return nullptr;
    }
    return _battle->_enemies[enemyTargetId2]->getSprite(
        targetInfo.enemySpriteId);
  }
  default:
    Engine::throw_("Unknown target type for room nav: " +
                   std::to_string(targetInfo.targetType));
    return nullptr;
  }
}
