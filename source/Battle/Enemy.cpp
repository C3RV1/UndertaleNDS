//
// Created by cervi on 05/10/2022.
//
#include "Battle/Enemy.hpp"
#include "Battle/Enemies/Dummy.hpp"
#include "Battle/Enemies/Flowey.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
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

void Enemy::renderHealth(int x, int y) {
  // TODO
}
