//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemies/Flowey.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include "Engine/WAV.hpp"
#include "Formats/utils.hpp"
#include <algorithm>
#include <memory>
#include <string>

std::unique_ptr<Enemy> getEnemy(u16 enemyId) {
  switch (enemyId) {
  case 0:
    return std::make_unique<Flowey>();
  case 1:
    return std::make_unique<Dummy>();
  default: {
    std::string msg = "Couldn't create enemy: " + std::to_string(enemyId);
    nocashMessage(msg.c_str());
    return nullptr;
  }
  }
}

void Enemy::loadName(int enemyId) {
  char buffer[100];
  sprintf(buffer, "nitro:/data/enemies/name%d.txt", enemyId);
  FILE *enemyNameFile = fopen(buffer, "rb");
  if (enemyNameFile) {
    int len = str_len_file(enemyNameFile, '\n');
    _name.resize(len);
    fread(&_name[0], len, 1, enemyNameFile);
    fseek(enemyNameFile, 1, SEEK_CUR);
  } else {
    sprintf(buffer, "Error opening enemy name %d", enemyId);
    nocashMessage(buffer);
  }
  fclose(enemyNameFile);
}

void Enemy::loadActText(int textId) {
  std::string buffer =
      "nitro:/data/battle_act_txt/" + std::to_string(textId) + ".txt";
  FILE *actTextFile = fopen(buffer.c_str(), "rb");
  if (actTextFile) {
    int len = str_len_file(actTextFile, '@');
    _actText.resize(len);
    fread(&_actText[0], len, 1, actTextFile);
    fseek(actTextFile, 1, SEEK_CUR);
  } else {
    buffer = "Error opening battle act " + std::to_string(textId);
    Engine::throw_(buffer);
  }
  fclose(actTextFile);
}

void Enemy::loadDamageSprites(int damage) {
  _damageSpriteCounter = 0;
  _slashSpr.loadTexture(
      Engine::textureManager.loadTexture("battle/spr_slice_o"));
  _damageNumbersSpr.clear();
  if (damage == 0) {
    Engine::Sprite missSpr{Engine::AllocatedOAM};
    missSpr.loadTexture(Engine::textureManager.loadTexture("battle/miss_text"));
    _damageNumbersSpr.push_back(std::move(missSpr));
    return;
  }

  while (damage > 0) {
    Engine::Sprite numSpr{Engine::AllocatedOAM};
    numSpr.loadTexture(
        Engine::textureManager.loadTexture("battle/damage_numbers"));
    numSpr.setFrame(damage % 10);
    _damageNumbersSpr.push_back(numSpr);
    damage /= 10;
  }
}

void Enemy::doSlash(s32 x, s32 y, int counter, int maxCounter) {
  if (_slashSpr._texture == nullptr)
    return;
  if (counter == 0) {
    auto lazSnd = std::make_shared<Audio2::WAV>();
    lazSnd->load("snd_laz.wav");
    Audio2::audioManager.play(std::move(lazSnd));
  }
  _slashSpr.setShown(counter < maxCounter);
  _slashSpr._wx = x;
  _slashSpr._wy = y;
}

void Enemy::doDamageNumbers(s32 x, s32 y, int counter, int maxCounter) {
  if (_damageNumbersSpr.empty())
    return;
  if (_damageNumbersSpr[0]._texture == nullptr)
    return;
  int w = _damageNumbersSpr[0]._texture->getWidth();
  int h = _damageNumbersSpr[0]._texture->getHeight();
  int totalW = _damageNumbersSpr.size() * w;

  x += (totalW << 8) / 2 - (w << 8);
  y -= (h << 8) / 2;
  if (counter < maxCounter / 3) {
    // Acceleration from 0 to 1 second.
    // y = y0 + v0*t + 1/2a*t^2
    // t = counter / (maxCounter / 3) t = 3*counter / maxCounter
    y += (kDamageNumSpeedY * counter * 3) / (maxCounter);
    y += (kDamageNumAccY * counter * counter * 3 * 3) /
         (2 * maxCounter * maxCounter);
  }

  for (auto &s : _damageNumbersSpr) {
    if (counter < maxCounter)
      s.setShown(true);
    else
      s.setShown(false);
    s._wx = x;
    s._wy = y;
    x -= w << 8;
  }
}

void Enemy::doRenderHealth(int x, int y, int counter, int maxCounter) {}

bool Enemy::defaultDamageAnimation(s32 x, s32 y, int width, int height) {
  if (_damageSpriteCounter <= 10 * 6)
    doSlash(x + (width << 8) / 2, y + (height << 8) / 2, _damageSpriteCounter,
            10 * 6);
  else if (_damageSpriteCounter <= 10 * 6 + 90) {
    doDamageNumbers(x + (width << 8) / 2, y, _damageSpriteCounter - 10 * 6, 90);
    doRenderHealth(x + (width << 8) / 2, y, _damageSpriteCounter - 10 * 6, 90);
  } else
    return true;
  _damageSpriteCounter++;
  return false;
}
