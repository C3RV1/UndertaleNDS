//
// Created by cervi on 29/08/2022.
//

#ifndef UNDERTALE_CUTSCENE_HPP
#define UNDERTALE_CUTSCENE_HPP

#include "Dialogue.hpp"
#include "SaveMenu.hpp"
#include "Waiting.hpp"
#include <cstdio>
#include <nds.h>
#include "Formats/utils.hpp"
#include "Battle/Battle.hpp"
#include "Fader.hpp"

class Room;

class Cutscene {
public:
  explicit Cutscene(u16 cutsceneId, u16 roomId, Room* room);
  bool checkHeader();
  void update();
  bool runCommands();
  bool runCommand(u8 cmd);
  u16 _cutsceneId;
  u16 _roomId;
  std::unique_ptr<Dialogue> _cDialogue = nullptr;
  std::unique_ptr<SaveMenu> _cSaveMenu = nullptr;
  std::unique_ptr<Battle> _cBattle = nullptr;
  Room* _room;
  Waiting _waiting;
  Fader _fader;

private:
  bool _flag = false;
  BufferReader _commandData;
};


#endif // UNDERTALE_CUTSCENE_HPP
