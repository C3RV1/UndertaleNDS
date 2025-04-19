//
// Created by cervi on 09/10/2023.
//
#include "Battle/FlavorTextDialogue.hpp"
#include "Battle/Battle.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include <memory>

FlavorTextDialogue::FlavorTextDialogue(u16 textId, std::string typeSnd,
                                       std::string font, u16 framesPerLetter)
    : DialogueLeftAligned(textId, 30 << 8, 22 << 8, nullptr, "", "", typeSnd,
                          font, framesPerLetter, Engine::textMain,
                          Engine::Allocated3D) {
  globalBattle->hide();
}

FlavorTextDialogue::FlavorTextDialogue(std::string text)
    : DialogueLeftAligned(30 << 8, 22 << 8, text, "SND_TXT2.wav",
                          "fnt_maintext.font", 2, Engine::textMain,
                          Engine::Allocated3D) {
  globalBattle->hide();
}

void FlavorTextDialogue::setShown(bool shown) {}

void FlavorTextDialogue::doRedraw() {
  DialogueLeftAligned::doRedraw();
  globalBattle->drawRect();
}

bool FlavorTextDialogue::update() {
  if (!globalBattle->moveInBattleRect())
    return false;
  if (!DialogueLeftAligned::update())
    return false;
  return true;
}
