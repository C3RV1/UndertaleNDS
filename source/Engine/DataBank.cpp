#include "Engine/DataBank.hpp"
#include "Engine/Engine.hpp"
#include "Formats/CBNK.hpp"
#include "Formats/utils.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

void DataBank::load(std::string path) {
  FILE *f = fopen(path.c_str(), "rb");
  if (!f) {
    std::string buffer = "Error opening text bank " + path;
    Engine::throw_(buffer);
  }

  u32 fileSizeWithoutHeader;

  if (!checkHeader(f, fileSizeWithoutHeader)) {
    std::string buffer = "Error header text bank " + path;
    Engine::throw_(buffer);
  }

  auto data = std::unique_ptr<u32[]>(new u32[(fileSizeWithoutHeader + 3) / 4]);
  fread(data.get(), fileSizeWithoutHeader, 1, f);
  fclose(f);

  u32 decompressedSize = data[0] >> 8;

  auto dataDecompressed = std::unique_ptr<u8[]>(new u8[decompressedSize]);
  decompress(data.get(), dataDecompressed.get(), DecompressType::LZ77);

  data = nullptr;

  u32 endTable = loadFileTable(dataDecompressed.get());

  _data = std::unique_ptr<u8[]>(new u8[decompressedSize - endTable]());
  memcpy(_data.get(), dataDecompressed.get() + endTable,
         decompressedSize - endTable);
}

bool DataBank::checkHeader(FILE *f, u32 &fileSize) {
  char header[4];
  char expectedHeader[4] = {'C', 'B', 'N', 'K'};

  fread(header, 4, 1, f);
  if (memcmp(header, expectedHeader, 4) != 0) {
    return false;
  }

  fread(&fileSize, 4, 1, f);
  long pos = ftell(f);
  fseek(f, 0, SEEK_END);
  u32 size = ftell(f);
  fseek(f, pos, SEEK_SET);

  if (size != fileSize) {
    return false;
  }

  u32 version;
  fread(&version, 4, 1, f);

  if (version != CBNK::version) {
    return false;
  }

  fileSize -= 3 * 4;
  return true;
}

u32 DataBank::loadFileTable(u8 *data) {
  u32 pos = 0;

  u32 count = *(u32 *)data;
  pos += 4;

  char buffer[32];
  u32 startPos, length;

  nocashMessage(
      ("Loading table with " + std::to_string(count) + " elements").c_str());
  for (u32 i = 0; i < count; i++) {
    memcpy(buffer, &data[pos], 32);
    nocashMessage(buffer);
    pos += 32;
    startPos = *(u32 *)(&data[pos]);
    pos += 4;
    length = *(u32 *)(&data[pos]);
    pos += 4;

    auto added = _fileTable.insert({{buffer}, {startPos, length}}).first;
    nocashMessage(("Added " + added->first + ": " +
                   std::to_string(added->second.first) + ", " +
                   std::to_string(added->second.second))
                      .c_str());
  }
  return pos;
}

std::string DataBank::getText(std::string textPath) {
  auto textKV = _fileTable.find(textPath);
  if (textKV == _fileTable.end()) {
    std::string buffer = "Couldn't find text: " + textPath;
    Engine::throw_(buffer);
  }
  auto textPosLen = textKV->second;
  return std::string(&_data[textPosLen.first],
                     &_data[textPosLen.first + textPosLen.second]);
}

const u8 *DataBank::getFile(std::string textPath) {
  auto textKV = _fileTable.find(textPath);
  if (textKV == _fileTable.end()) {
    std::string buffer = "Couldn't find text: " + textPath;
    Engine::throw_(buffer);
  }
  auto textPosLen = textKV->second;
  return &_data[textPosLen.first];
}

u32 DataBank::getSize(std::string textPath) {
  auto textKV = _fileTable.find(textPath);
  if (textKV == _fileTable.end()) {
    std::string buffer = "Couldn't find text: " + textPath;
    Engine::throw_(buffer);
  }
  auto textPosLen = textKV->second;
  return textPosLen.second;
}

DataBank textBank;
