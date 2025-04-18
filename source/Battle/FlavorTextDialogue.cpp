//
// Created by cervi on 09/10/2023.
//
#include "Battle/FlavorTextDialogue.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/Texture.hpp"
#include <memory>

FlavorTextDialogue::FlavorTextDialogue(u16 textId)
    : DialogueLeftAligned(textId, 30 << 8, 22 << 8, nullptr, "", "",
                          "SND_TXT2.WAV", "fnt_maintext.font", 2,
                          Engine::textMain, Engine::Allocated3D) {

  _flavorTextSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_flavorTextSpr, "battle/flavor_text_box");
  _flavorTextSpr->_wx = 22 << 8;
  _flavorTextSpr->_wy = 18 << 8;
}

FlavorTextDialogue::FlavorTextDialogue(std::string text)
    : DialogueLeftAligned(30 << 8, 22 << 8, text, "SND_TXT2.wav",
                          "fnt_maintext.font", 2, Engine::textMain,
                          Engine::Allocated3D) {

  _flavorTextSpr = std::make_shared<Engine::Sprite>(Engine::Allocated3D);
  Engine::spriteLoadTexture(_flavorTextSpr, "battle/flavor_text_box");
  _flavorTextSpr->_wx = 22 << 8;
  _flavorTextSpr->_wy = 18 << 8;
}

void FlavorTextDialogue::setShown(bool shown) {
  Engine::spriteSetShown(_flavorTextSpr, shown);
}
