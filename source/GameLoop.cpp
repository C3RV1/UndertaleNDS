#include "GameLoop.hpp"
#include "Engine/Engine.hpp"
#include "Room/Room.hpp"
#include "Room/InGameMenu.hpp"
#include "Fader.hpp"
#include <memory>

static std::unique_ptr<struct ScheduledRoom> nextRoom;

void scheduleRoom(u16 roomId, u16 spawnX, u16 spawnY) {
  nextRoom = std::make_unique<struct ScheduledRoom>();
  nextRoom->roomId = roomId;
  nextRoom->spawnX = spawnX;
  nextRoom->spawnY = spawnY;
}

void changeRoom(std::unique_ptr<Room>& room,
                std::unique_ptr<struct ScheduledRoom> nextRoom) {
  auto igm = std::move(room->_ingame_menu);
  Fader fader;

  fader.startFade(FadeType::FADE_OUT, FadeScreen::MAIN);
  while (!fader.fadeFinished()) {
    room->updateDrawPositions();
    fader.update();
    Engine::tick();
    room->draw();
  }
  
  igm->show();
  room = std::make_unique<Room>(
      nextRoom->roomId, std::make_pair(nextRoom->spawnX, nextRoom->spawnY),
      std::move(igm), std::move(room->_save));

  fader.startFade(FadeType::FADE_IN, FadeScreen::MAIN);
  while (!fader.fadeFinished()) {
    room->updateDrawPositions();
    fader.update();
    Engine::tick();
    room->draw();
  }
}

void runGameLoop(std::unique_ptr<SaveData> save) {
  auto igm = std::make_unique<InGameMenu>(save.get());
  igm->load();
  igm->show();

  auto room =
      std::make_unique<Room>(save->lastSavedRoom, std::nullopt, std::move(igm),
                             std::move(save));

  for (;;) {
    room->update();
    room->updateDrawPositions();
    Engine::tick();
    room->draw();

    if (nextRoom)
      changeRoom(room, std::move(nextRoom));
  }
}
