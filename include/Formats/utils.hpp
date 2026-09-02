//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_UTILS_HPP
#define UNDERTALE_UTILS_HPP

#include <cstdio>
#include <vector>
#include <string>
#include "nds.h"

[[deprecated]] int str_len_file(FILE *f, char terminator);

template <typename T>
std::string to_string_pad(T val, size_t len, char pad_char) {
  auto s = std::to_string(val);
  return std::string(len - std::min(s.length(), len), pad_char) + s;
}

class BufferReader {
public:
  void close();
  void openFromFile(FILE *f, size_t size);
  bool isOpen();
  void read(void *ptr, size_t size);
  [[deprecated]] void strncpy(char *ptr, size_t max_size);
  std::string readstring();
  long tell() const;
  void seek(long where);
  size_t size() const;
  bool at_end() const;
private:
  bool _opened = false;
  std::vector<u8> data;
  std::vector<u8>::const_iterator pos;
};

#endif // UNDERTALE_UTILS_HPP
