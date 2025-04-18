//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_ROOM_FILE_HPP
#define UNDERTALE_ROOM_FILE_HPP

#include <nds.h>
#include <string>
#include <vector>

struct ROOMHeader {
  char header[4] = {'R', 'O', 'O', 'M'};
  u32 fileSize = 0;

  u32 version = 10;
  static constexpr u32 version_expected = 10;
};

struct ROOMExit {
  u8 exitType = 0; // 0 side, 1 rect
  u16 roomId = 0;
  u16 spawnX = 0, spawnY = 0;
  u16 x = 0, y = 0, w = 0, h = 0; // rect
  u8 side = 0;                    // side 0-3, up, down, left, right
};

struct ROOMExits {
  std::vector<ROOMExit> roomExits;
};

enum class ROOMSpriteAction {
  NONE = 0,
  CUTSCENE = 1,
  PROXIMITY = 2,
  PARALLAX = 3,
  PUSHABLE = 4
};

struct ROOMSpriteData {
  std::string path;
  u16 x = 0, y = 0;
  std::string animation;
  u8 interactAction = 0; // ROOMSpriteAction

  u16 cutsceneId = 0; // only when interactAction == CUTSCENE

  // only when interactAction == PROXIMITY
  u16 distance = 0;
  std::string closeAnim;

  // only when interactAction == PARALLAX
  s32 parallax_x, parallax_y;

  // only when interactAction == PUSHABLE
  u16 valid_rect_x, valid_rect_y, valid_rect_w, valid_rect_h;
  u16 goal_x, goal_y;
  u16 goal_cutscene_id, goal_flag_id;
  u16 goal_flag_bit;
  bool stop_on_goal;
};

struct ROOMSprites {
  std::vector<ROOMSpriteData> roomSprites;
};

struct ROOMCollider {
  u16 x = 0, y = 0, w = 0, h = 0;
  u8 colliderAction = 0; // 0 - wall, 1 - cutscene
  bool enabled = true;
  u16 cutsceneId = 0; // only when colliderAction == 1
};

struct ROOMColliders {
  std::vector<ROOMCollider> roomColliders;
};

struct ROOMPartCondition {
  u16 flagId = 0;
  u8 cmpOperator = 0; // Same as cutscene cmp, but bit 4 is flip
  u16 cmpValue = 0;
};

struct ROOMPart {
  u32 lengthBytes = 0;
  u8 conditionCount = 0;
  ROOMPartCondition *conditions = nullptr;
  std::string roomBg;
  std::string musicBg;
  u8 musicVolume;
  u16 spawnX = 0, spawnY = 0;
  ROOMExits roomExits;
  ROOMSprites roomSprites;
  ROOMColliders roomColliders;
};

struct ROOMFile {
  ROOMHeader header;
  u8 partCount = 0;
  ROOMPart *parts = nullptr;
};

#endif // UNDERTALE_ROOM_FILE_HPP
