//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemies/Flowey.hpp"
#include "Battle/Enemies/Froggit.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Engine/WAV.hpp"
#include "Engine/math.hpp"
#include "Formats/utils.hpp"
#include <memory>
#include <string>

Enemy::Enemy() {
  _slashSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
}

std::unique_ptr<Enemy> getEnemy(u16 enemyId) {
  switch (enemyId) {
  case 0:
    return std::make_unique<Flowey>();
  case 1:
    return std::make_unique<Dummy>();
  case 2:
    return std::make_unique<Froggit>(true);
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
  Engine::spriteLoadTexture(_slashSpr, "battle/spr_slice_o");
  _damageNumbersSpr.clear();

  if (damage == 0) {
    auto missSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
    Engine::spriteLoadTexture(missSpr, "battle/miss_text");
    _damageNumbersSpr.push_back(std::move(missSpr));
    _damageAnimStep = EnemyDamageAnimationStep::DAMAGE_NUMBERS;
    _missed = true;
    return;
  }

  _damageAnimStep = EnemyDamageAnimationStep::SLASH;
  _missed = false;
  while (damage > 0) {
    auto numSpr = std::make_shared<Engine::Sprite>(Engine::AllocatedOAM);
    Engine::spriteLoadTexture(numSpr, "battle/damage_numbers");
    numSpr->setFrame(damage % 10);
    _damageNumbersSpr.push_back(numSpr);
    damage /= 10;
  }
}

void Enemy::doSlash(s32 x, s32 y, int counter, int maxCounter) {
  if (_slashSpr->_texture == nullptr)
    return;
  if (counter == 0) {
    auto lazSnd = std::make_shared<Audio2::WAV>();
    lazSnd->load("snd_laz.wav");
    Audio2::audioManager.play(std::move(lazSnd));
  }
  Engine::spriteSetShown(_slashSpr, counter < maxCounter);
  _slashSpr->_wx = x;
  _slashSpr->_wy = y;
}

void Enemy::slashFinished() {
  auto damageSnd = std::make_shared<Audio2::WAV>();
  damageSnd->load("snd_damage.wav");
  Audio2::audioManager.play(std::move(damageSnd));
}

void Enemy::doDamageNumbers(s32 x, s32 y, int counter, int maxCounter) {
  if (_damageNumbersSpr.empty())
    return;
  if (_damageNumbersSpr[0]->_texture == nullptr)
    return;
  int w = _damageNumbersSpr[0]->_texture->getWidth();
  int h = _damageNumbersSpr[0]->_texture->getHeight();
  int totalW = _damageNumbersSpr.size() * w;

  x += (totalW << 8) / 2 - (w << 8);
  y -= (kHpBarHeigth + 2 + h + 2) << 8;
  if (counter < maxCounter / 3 && !_missed) {
    // Acceleration from 0 to 1 second.
    // y = y0 + v0*t + 1/2a*t^2
    // t = counter / (maxCounter / 3) t = 3*counter / maxCounter
    y += (kDamageNumSpeedY * counter * 3) / (maxCounter);
    y += (kDamageNumAccY * counter * counter * 3 * 3) /
         (2 * maxCounter * maxCounter);
  }

  for (auto &s : _damageNumbersSpr) {
    if (counter < maxCounter)
      Engine::spriteSetShown(s, true);
    else
      Engine::spriteSetShown(s, false);
    s->_wx = x;
    s->_wy = y;
    x -= w << 8;
  }
}

void Enemy::doRenderHealth(int x, int y, int counter, int maxCounter) {
  // Ease-out cubic: 1-(1-x)^3
  //                 1-(1-c/m)^3
  //                 1-((m-c)^3/m^3)
  //                 (m^3-(m-c)^3)/m^3
  int maxUsed = maxCounter / 2;
  int counterUsed = counter > maxUsed ? maxUsed : counter;

  int hpDraw = easeOutCubic(_prevHp, _hp, counterUsed, maxUsed);

  x -= kHpBarWidth / 2;
  y -= kHpBarHeigth + 2;
  if (counter == maxCounter)
    Engine::textSub.clearRect(x, y, kHpBarWidth, kHpBarHeigth);
  else
    Engine::textSub.drawHpBar(hpDraw, _maxHp, x, y, kHpBarWidth, kHpBarHeigth);
}

void Enemy::doShake(int counter, int maxCounter) {
  s32 angle = kShakeMaxAngle * counter / maxCounter;
  s16 sinAngle = sinLerp(angle % DEGREES_IN_CIRCLE); // 4.12 fixed
  s32 amplitude = easeOutCubic(kShakeAmplitude, 0, counter, maxCounter);
  s32 dx = (sinAngle * amplitude) >> (8 + 12 - 8); // 24.8 fixed

  shakeSprites(dx);
}

bool Enemy::defaultDamageAnimation(s32 x, s32 y, int width, int height) {
  if (_damageAnimStep == EnemyDamageAnimationStep::SLASH) {
    doSlash(x + (width << 8) / 2, y + (height << 8) / 2, _damageSpriteCounter,
            kSlashFrames);
    if (_damageSpriteCounter == kSlashFrames) {
      slashFinished();
      _damageSpriteCounter = 0;
      _damageAnimStep = EnemyDamageAnimationStep::DAMAGE_NUMBERS;
    }
  } else if (_damageAnimStep == EnemyDamageAnimationStep::DAMAGE_NUMBERS) {
    doDamageNumbers(x + (width << 8) / 2, y, _damageSpriteCounter,
                    kDamageNumFrames);
    if (!_missed) {
      doRenderHealth((x >> 8) + (width) / 2, y >> 8, _damageSpriteCounter,
                     kDamageNumFrames);
      doShake(_damageSpriteCounter, kDamageNumFrames);
    }
    if (_damageSpriteCounter == kDamageNumFrames) {
      damageAnimationFinished();
      return true;
    }
  }
  _damageSpriteCounter++;
  return false;
}
