#include "Formats/utils.hpp"
#include <cstdio>

int str_len_file(FILE *f, char terminator) {
  if (f == nullptr)
    return -1;
  long pos = ftell(f);
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, pos, SEEK_SET);
  int count = -1;
  char byte = 0;
  do {
    fread(&byte, 1, 1, f);
    count += 1;
  } while (byte != terminator && ftell(f) != len);
  fseek(f, pos, SEEK_SET);
  return count;
}

void BufferReader::openFromFile(FILE* f, size_t size) {
  data.resize(size);
  fread(&data[0], 1, size, f);
  pos = data.cbegin();
  _opened = true;
}

void BufferReader::read(void *ptr, size_t size) {
  if (!_opened)
    return;
  memcpy(ptr, pos.base(), size);
  pos += size;
}

void BufferReader::strncpy(char *ptr, size_t max_size) {
  if (!_opened) {
    *ptr = 0;
    return;
  }
  size_t i = 0;
  while (i < max_size - 1 && *pos != '\0' && pos != data.cend()) {
    i++;
    *(ptr++) = (*pos++);
  }
  *ptr = '\0';
  
  while (pos != data.cend() && *(pos++) != '\0') [[unlikely]];
}

std::string BufferReader::readstring() {
  if (!_opened)
    return "";
  std::string str;
  while (*pos != '\0' && pos != data.cend())
    str.push_back(*(pos++));
  pos++;
  return str;
}

long BufferReader::tell() const {
  return std::distance(data.cbegin(), pos);
}

void BufferReader::seek(long where) {
  if (!_opened)
    return;
  pos = data.cbegin() + where;
}

void BufferReader::close() {
  _opened = false;
  data.clear();
}

bool BufferReader::at_end() const {
  if (!_opened)
    return true;
  return pos == data.cend();
}

size_t BufferReader::size() const {
  if (!_opened)
    return 0;
  return data.size();
}
