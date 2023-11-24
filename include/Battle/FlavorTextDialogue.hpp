//
// Created by cervi on 09/10/2023.
//

#ifndef UNDERTALE_FLAVORTEXTDIALOGUE_HPP
#define UNDERTALE_FLAVORTEXTDIALOGUE_HPP

#include "Cutscene/Dialogue.hpp"

class FlavorTextDialogue : public DialogueLeftAligned {
public:
    explicit FlavorTextDialogue(u16 textId);
    explicit FlavorTextDialogue(std::string text);

    void setShown(bool shown);

protected:
    Engine::Texture _flavorTextTex;
    Engine::Sprite _flavorTextSpr;
};

#endif //UNDERTALE_FLAVORTEXTDIALOGUE_HPP
