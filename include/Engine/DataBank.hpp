//
// Created by cervi on 22/04/2025
//

#ifndef DATA_BANK_HPP
#define DATA_BANK_HPP

#include <map>
#include <memory>
#include <nds.h>
#include <string>

class DataBank {
public:
  void load(std::string path);
  std::string getText(std::string path);
  const u8 *getFile(std::string path);
  u32 getSize(std::string path);

private:
  bool checkHeader(FILE *f, u32 &fileSize);
  u32 loadFileTable(u8 *data);
  std::map<std::string, std::pair<u32, u32>> _fileTable;
  std::unique_ptr<u8[]> _data;
};

extern DataBank textBank;

#endif
