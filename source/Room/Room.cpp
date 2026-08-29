//
// Created by cervi on 27/08/2022.
//

#include "Room/Room.hpp"
#include "ConditionalFile/RoomConditionalFile.hpp"
#include "Cutscene/Cutscene.hpp"
#include "Engine/Background.hpp"
#include "Engine/Engine.hpp"
#include "Engine/Sprite.hpp"
#include "Engine/WAV.hpp"
#include "Formats/utils.hpp"
#include "Room/Camera.hpp"
#include "Room/InGameMenu.hpp"
#include "Room/Player.hpp"
#include "Save.hpp"
#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <utility>

Room::Room(int roomId, std::optional<std::pair<u16, u16>> spawnCoords,
           std::unique_ptr<InGameMenu> ingame_menu,
           std::unique_ptr<SaveData> save)
    : _roomId(roomId), _nav(this), _player(this), _camera(this),
      _ingame_menu(std::move(ingame_menu)), _save(std::move(save)) {
  
  std::string buffer = "nitro:/new_rooms/room" + std::to_string(roomId) + ".room";
  
  FILE *f = fopen(buffer.c_str(), "rb");
  if (f == nullptr) {
    buffer = "Error opening room " + std::to_string(roomId);
    Engine::throw_(buffer);
  }
  
  loadRoom(f);
  fclose(f);
  
  for (int i = 210; i <= 219; i++) {
    _save->flags[i] = 0; // clear room specific flags
  }

  _bg.loadPath(_roomData._roomBg);

  int bgLoad = _bg.loadBgExtendedMain(512 / 8);
  if (bgLoad != 0)
    Engine::throw_("Error loading room bg. Error Code: " + std::to_string(bgLoad));

  if (!_roomData._musicPath.empty()) {
    bool musicChange =
        _roomData._musicPath != Audio2::cBGMusic->getFilename() ||
        !Audio2::cBGMusic->getPlaying();
    Audio2::cBGMusic->setVolume(_roomData._musicVolume);
    if (musicChange) {
      Audio2::playBGMusic(_roomData._musicPath, true);
    }
  } else {
    Audio2::stopBGMusic();
  }

  loadSprites();
  
  if (spawnCoords) {
    _player._spr->_wx = spawnCoords->first << 8;
    _player._spr->_wy = spawnCoords->second << 8;
  }
  else {
    _player._spr->_wx = _roomData._spawnX << 8;
    _player._spr->_wy = _roomData._spawnY << 8;
  }
  _camera.updatePosition(true, _player);
}

void Room::loadRoom(FILE *f) {
  RoomHeader header;
  fread(header.header, 4, 1, f);
  char expectedHeader[4] = {'R', 'O', 'O', 'M'};

  if (memcmp(expectedHeader, header.header, 4) != 0) {
    std::string buffer = "Error loading room #r" + std::to_string(_roomId) +
                         "#x: Invalid header.";
    Engine::throw_(buffer);
  }

  fread(&header.fileSize, 4, 1, f);
  long pos = ftell(f);
  fseek(f, 0, SEEK_END);
  u32 size = ftell(f);
  fseek(f, pos, SEEK_SET);

  if (header.fileSize != size) {
    std::string buffer = "Error loading spr #r" + std::to_string(_roomId) +
                         "#x: File size doesn't match (expected: " +
                         std::to_string(header.fileSize) +
                         ", actual: " + std::to_string(size) + ")";
    Engine::throw_(buffer);
  }

  fread(&header.version, 4, 1, f);
  if (header.version != RoomHeader::version_expected) {
    std::string buffer = "Error loading room #r" + std::to_string(_roomId) +
                         "#x: Invalid version (expected: 10, actual: " +
                         std::to_string(header.version) + ")";
    Engine::throw_(buffer);
  }

  BufferReader rdr;
  rdr.openFromFile(f, header.fileSize - ftell(f));
  
  _roomData.read(&rdr, _save.get());

  for (auto& exit : _roomData._roomExits) {
    switch(exit._exitSide) {
    case 0:
      _exitTop = &exit;
      break;
    case 1:
      _exitBtm = &exit;
      break;
    case 2:
      _exitLeft = &exit;
      break;
    case 3:
      _exitRight = &exit;
      break;
    default:
      Engine::throw_("Error loading room #r" + std::to_string(_roomId) +
                     "#x: Invalid exit side " + std::to_string(exit._exitSide));
    }
  }
}

void Room::loadSprites() {
  for (auto const &roomSprite : _roomData._roomSprites) {
    if (roomSprite._sprId != 0 && _sprites.count(roomSprite._sprId) > 0)
      Engine::throw_("Room: Duplicate spr id != 0: " +
                     std::to_string(roomSprite._sprId));
    _sprites
        .emplace(std::piecewise_construct, std::make_tuple(roomSprite._sprId),
                 std::make_tuple(Engine::Allocated3D, this))
        ->second.load(roomSprite);
  }
}

void Room::draw() {
  _player.draw();
  for (auto &sprite : _sprites) {
    sprite.second.draw();
  }
}

void Room::update() {
  if (_cutscene != nullptr) {
    _cutscene->update();
    if (_cutscene->runCommands()) {
      _cutscene = nullptr;
      _ingame_menu->show();
      _player.set_player_control(true);
      _camera._manual = false;
    } else if (!_cutscene->_fader.fadeFinished())
      return;
  }

  if (_pushed)
    return;
  
  _player.update();
  _nav.update();
  for (auto & sprite : _sprites) {
    sprite.second.update();
  }
  
  _camera.updatePosition(false, _player);
  _ingame_menu->update(*this);
}

void Room::push() {
  _pushed = true;
  
  Engine::spritePush(_player._spr);
  for (auto &sprite : _sprites) {
    Engine::spritePush(sprite.second._spr);
  }

  _ingame_menu->unload();
  
  Engine::textMain.clear();
  Engine::textSub.clear();
}

void Room::pop() {
  _pushed = false;
  
  Engine::textMain.clear();
  Engine::textSub.clear();
  
  _bg.loadPath(_roomData._roomBg);
  Engine::spritePop(_player._spr);
  Engine::clearSub();

  int bgLoad = _bg.loadBgExtendedMain(512 / 8);
  if (bgLoad != 0)
    Engine::throw_("Error loading room bg: %d" + std::to_string(bgLoad));

  for (auto &sprite : _sprites) {
    Engine::spritePop(sprite.second._spr);
  }

  _ingame_menu->load();

  _camera.updatePosition(true, _player);
  draw();
}

