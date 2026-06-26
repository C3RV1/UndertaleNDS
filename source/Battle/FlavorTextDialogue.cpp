//
// Created by cervi on 09/10/2023.
//
#include "Battle/FlavorTextDialogue.hpp"
#include "Battle/Battle.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include <memory>

FlavorTextDialogue::FlavorTextDialogue(Battle *battle, u16 cutsceneId,
                                       u16 roomId, u16 textId,
                                       std::string typeSnd, std::string font,
                                       u16 framesPerLetter)
    : DialogueLeftAligned(battle->_save, cutsceneId, roomId, textId, nullptr,
                          "", "", typeSnd, font, framesPerLetter,
                          Engine::textMain, Engine::Allocated3D, 30 << 8,
                          22 << 8),
      _battle(battle) {
  _battle->hide();
  _battle->drawRect();
}

FlavorTextDialogue::FlavorTextDialogue(Battle *battle, std::string text)
    : DialogueLeftAligned(battle->_save, text, nullptr, "", "", "SND_TXT2.wav",
                          "fnt_maintext.font", 2, Engine::textMain,
                          Engine::Allocated3D, 30 << 8, 22 << 8),
      _battle(battle) {
  _battle->hide();
  _battle->drawRect();
}

void FlavorTextDialogue::setShown(bool shown) { _battle->drawRect(); }

void FlavorTextDialogue::doRedraw() {
  DialogueLeftAligned::doRedraw();
  _battle->drawRect();
}

void FlavorTextDialogue::onClear() {
  DialogueLeftAligned::onClear();
  _battle->drawRect();
}

bool FlavorTextDialogue::update() {
  if (!_battle->moveInBattleRect())
    return false;
  if (!DialogueLeftAligned::update())
    return false;
  return true;
}
