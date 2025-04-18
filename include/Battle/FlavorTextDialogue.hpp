//
// Created by cervi on 09/10/2023.
//

#ifndef UNDERTALE_FLAVORTEXTDIALOGUE_HPP
#define UNDERTALE_FLAVORTEXTDIALOGUE_HPP

#include "Cutscene/Dialogue.hpp"
#include "Engine/Sprite.hpp"
#include <memory>

class FlavorTextDialogue : public DialogueLeftAligned {
public:
  explicit FlavorTextDialogue(u16 textId);
  explicit FlavorTextDialogue(std::string text);

  void setShown(bool shown);

protected:
  std::shared_ptr<Engine::Sprite> _flavorTextSpr;
};

#endif // UNDERTALE_FLAVORTEXTDIALOGUE_HPP
