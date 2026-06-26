#include "Battle/BattleNavigation.hpp"
#include "Battle/Battle.hpp"
#include "Battle/BattleAction.hpp"
#include <memory>
#include <string>

BattleNavigation::BattleNavigation(Battle* battle): _battle(battle) {}

void BattleNavigation::spawn_sprite(const std::string &path, s32 x, s32 y,
                                  s32 layer) {
  auto newSprite = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
  newSprite->_wx = x;
  newSprite->_wy = y;
  newSprite->_layer = layer;
  Engine::spriteLoadTexture(newSprite, path);
  Engine::spriteSetShown(newSprite, true);

  _battle->_sprites.push_back(std::move(newSprite));
}

void BattleNavigation::unload_sprite(s8 sprId) {
  u8 sprId2;
  if (sprId < 0)
    sprId2 = _battle->_sprites.size() + sprId;
  else
    sprId2 = sprId;

  if (sprId2 >= _battle->_sprites.size())
    return;

  _battle->_sprites.erase(_battle->_sprites.begin() + sprId2);
}

std::shared_ptr<Engine::Sprite>
BattleNavigation::getTarget(const TargetInfo &targetInfo) {
  TargetType targetType = static_cast<TargetType>(targetInfo.targetType);
  switch (targetType) {
  case TargetType::SPRITE: {
    u8 targetId2;
    if (targetInfo.targetId < 0)
      targetId2 = _battle->_sprites.size() + targetInfo.targetId;
    else
      targetId2 = targetInfo.targetId;
    if (targetId2 >= _battle->_sprites.size()) {
      nocashMessage("Error: target id outside of sprite count");
      return nullptr;
    }
    return _battle->_sprites[targetInfo.targetId];
  }
  case TargetType::ENEMY: {
    u8 enemyTargetId2;
    if (targetInfo.targetId < 0)
      enemyTargetId2 = _battle->_enemies.size() + targetInfo.targetId;
    else
      enemyTargetId2 = targetInfo.targetId;
    if (enemyTargetId2 >= _battle->_enemies.size()) {
      nocashMessage("Error: target id outside of enemy count");
      return nullptr;
    }
    return _battle->_enemies[enemyTargetId2]->getSprite(
        targetInfo.enemySpriteId);
  }
  default:
    nocashMessage(("Unknown target type for room nav: " +
                   std::to_string(targetInfo.targetType))
                      .c_str());
    return nullptr;
  }
}
