//
// Created by cervi on 09/10/2023.
//
#include "Battle/FlavorTextDialogue.hpp"
#include "Engine/Texture.hpp"

FlavorTextDialogue::FlavorTextDialogue(u16 textId)
    : DialogueLeftAligned(textId, 30 << 8, 25 << 8, nullptr, "", "",
                          "SND_TXT2.WAV", "fnt_maintext.font", 2,
                          Engine::textMain, Engine::Allocated3D),
      _flavorTextSpr(Engine::Allocated3D) {

  _flavorTextSpr.loadTexture(
      Engine::textureManager.loadTexture("battle/flavor_text_box"));
  _flavorTextSpr._wx = 22 << 8;
  _flavorTextSpr._wy = 18 << 8;
}

FlavorTextDialogue::FlavorTextDialogue(std::string text)
    : DialogueLeftAligned(30 << 8, 25 << 8, text, "SND_TXT2.wav",
                          "fnt_maintext.font", 2, Engine::textMain,
                          Engine::Allocated3D),
      _flavorTextSpr(Engine::Allocated3D) {

  _flavorTextSpr.loadTexture(
      Engine::textureManager.loadTexture("battle/flavor_text_box"));
  _flavorTextSpr._wx = 22 << 8;
  _flavorTextSpr._wy = 18 << 8;
}

void FlavorTextDialogue::setShown(bool shown) {
  _flavorTextSpr.setShown(shown);
}
