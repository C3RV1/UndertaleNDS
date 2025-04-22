//
// Created by cervi on 22/04/2025
//

#ifndef UNDERTALE_TBNK_HPP
#define UNDERTALE_TBNK_HPP

#include <map>
#include <nds.h>
#include <string>
#include <utility>

struct CBNK {
  char header[4] = {'C', 'B', 'N', 'K'};
  static constexpr u32 version = 1;
  u32 fileSize = 0;
  u8 *compressedData;
};

struct CBNKTable {
  u32 count;
  std::map<std::string, std::pair<u32, u32>> fileTable;
};

struct CBNKUncompressed {
  CBNKTable table;
  u8 *data;
};

#endif
