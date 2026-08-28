//
// Created by cervi on 25/08/2022.
//

#ifndef UNDERTALE_ROOM_HPP
#define UNDERTALE_ROOM_HPP

#include "ConditionalFile/RoomConditionalFile.hpp"
#include "Room/InGameMenu.hpp"
#include "RoomNavigation.hpp"
#include "Engine/Background.hpp"
#include "Room/RoomSprite.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include <cstdio>
#include <cstring>
#include <vector>
#include <map>
#include <optional>

class Cutscene;

class Room {
public:
  Room(int roomId, std::optional<std::pair<u16, u16>> spawnCoords,
       std::unique_ptr<InGameMenu> ingame_menu, std::unique_ptr<SaveData> save);
  void update();
  void draw();

  void push();
  bool _pushed = false;
  void pop();

  u16 _roomId;
  Engine::Background _bg;

  std::multimap<u16, RoomSprite> _sprites;

  RoomData _roomData;

  RoomSideExit *_exitTop = nullptr;
  RoomSideExit *_exitBtm = nullptr;
  RoomSideExit *_exitLeft = nullptr;
  RoomSideExit *_exitRight = nullptr;
  RoomNavigation _nav;

  Player _player;
  Camera _camera;
  std::unique_ptr<InGameMenu> _ingame_menu;
  std::unique_ptr<Cutscene> _cutscene = nullptr;

  std::unique_ptr<SaveData> _save;
  
private:
  void loadRoom(FILE *f);
  bool evaluateCondition(FILE *f);
  void loadSprites();
};

#endif // UNDERTALE_ROOM_HPP
