//
// Created by cervi on 09/10/2023.
//

#ifndef UNDERTALE_FLAVORTEXTDIALOGUE_HPP
#define UNDERTALE_FLAVORTEXTDIALOGUE_HPP

#include "Cutscene/Dialogue.hpp"

class FlavorTextDialogue : public DialogueLeftAligned {
public:
  explicit FlavorTextDialogue(u16 textId, std::string typeSnd, std::string font,
                              u16 framesPerLetter);
  explicit FlavorTextDialogue(std::string text);

  void setShown(bool shown);
  void doRedraw() final;
  void onClear() final;
  bool update() final;
};

#endif // UNDERTALE_FLAVORTEXTDIALOGUE_HPP
