//
// Created by cervi on 22/04/2025
//

#ifndef TEXT_BANK_HPP
#define TEXT_BANK_HPP

#include <map>
#include <memory>
#include <nds.h>
#include <string>

class TextBank {
public:
  void load(std::string path);
  std::string getText(std::string textPath);

private:
  bool checkHeader(FILE *f, u32 &fileSize);
  u32 loadFileTable(u8 *data);
  std::map<std::string, std::pair<u32, u32>> _fileTable;
  std::unique_ptr<u8[]> _data;
};

#endif
