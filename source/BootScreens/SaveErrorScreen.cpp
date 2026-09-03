#include "BootScreens/SaveErrorScreen.hpp"
#include "Engine/DataBank.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/TextBGManager.hpp"
#include "Cutscene/Dialogue.hpp"
#include "Save.hpp"

void runSaveErrorScreen(SaveData& save) {
  lcdMainOnTop();
  return;
  
  std::string dialogueText = textBank.getText("");
  std::string typeSnd = "";
  std::string fontTxt = "fnt_maintext.font";
  auto dialogue = std::make_unique<DialogueCentered>(
      &save, dialogueText, nullptr, "", "", typeSnd, fontTxt, 3, Engine::textSub,
      Engine::AllocatedOAM, "", 0, 0, "", "");
}
