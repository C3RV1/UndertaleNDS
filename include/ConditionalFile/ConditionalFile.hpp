//
// Created by cervi on 27/08/26
//
#ifndef UNDERTALE_CONDITIONAL_FILE_HPP
#define UNDERTALE_CONDITIONAL_FILE_HPP

#include "Formats/utils.hpp"
#include "Save.hpp"
#include <nds.h>
#include <optional>

class ConditionalObj {
public:
  virtual void read(BufferReader *rdr, SaveData *save) = 0;
  bool nextIsConditional(BufferReader *rdr);

private:
  u8 _lastUnconditionalCount = 0;
};

class Condition {
public:
  Condition(u16 flag, u8 cmp, u16 cmp_value);
  bool checkCondition(SaveData *save);
  bool hasMoreConditions();
  bool hasMoreVariations();

private:
  static constexpr u8 kFlipBit = 1 << 2;
  static constexpr u8 kHasNextConditionBit = 1 << 3;
  static constexpr u8 kHasNextVariationBit = 1 << 4;

  u16 _flag, _cmp_value;
  u8 _cmp;
};

Condition readCondition(BufferReader *rdr);

template <typename> struct tag {};

u8 readConditionalValue(tag<u8>, BufferReader *rdr, SaveData *save);
u16 readConditionalValue(tag<u16>, BufferReader *rdr, SaveData *save);
u32 readConditionalValue(tag<u32>, BufferReader *rdr, SaveData *save);
s8 readConditionalValue(tag<s8>, BufferReader *rdr, SaveData *save);
s16 readConditionalValue(tag<s16>, BufferReader *rdr, SaveData *save);
s32 readConditionalValue(tag<s32>, BufferReader *rdr, SaveData *save);
bool readConditionalValue(tag<bool>, BufferReader *rdr, SaveData *save);
std::string readConditionalValue(tag<std::string>, BufferReader *rdr,
                                 SaveData *save);

template <class T>
T readConditionalData(BufferReader *rdr, SaveData *save, ConditionalObj *obj) {
  if (!obj->nextIsConditional(rdr)) {
    return readConditionalValue(tag<T>{}, rdr, save);
  }

  std::optional<T> data = {};
  std::optional<Condition> c = {};
  do {
    bool areConditionsTrue = true;
    do {
      c = readCondition(rdr);
      areConditionsTrue &= c->checkCondition(save);
    } while (c->hasMoreConditions());

    if (areConditionsTrue && !data) {
      data = readConditionalValue(tag<T>{}, rdr, save);
    } else {
      T _ = readConditionalValue(tag<T>{}, rdr, save);
    }

  } while (c->hasMoreVariations());

  if (!data) {
    return readConditionalValue(tag<T>{}, rdr, save);
  } else {
    T _ = readConditionalValue(tag<T>{}, rdr, save);
  }

  return *data;
}

template <class T>
class VectorConditional : public std::vector<T>, public ConditionalObj {
public:
  void read(BufferReader *rdr, SaveData *save) override;
  static std::vector<T> readConditionalVector(BufferReader *rdr,
                                              SaveData *save);
};

template <class T>
void VectorConditional<T>::read(BufferReader *rdr, SaveData *save) {
  u16 num_elements;
  rdr->read(&num_elements, 2);
  for (size_t i = 0; i < num_elements; i++) {
    if (!nextIsConditional(rdr)) {
      this->push_back(readConditionalValue(tag<T>{}, rdr, save));
      continue;
    }

    std::optional<Condition> c = {};
    bool areConditionsTrue = true;
    do {
      c = readCondition(rdr);
      areConditionsTrue &= c->checkCondition(save);
    } while (c->hasMoreConditions());

    T data = readConditionalValue(tag<T>{}, rdr, save);
    if (areConditionsTrue)
      this->push_back(data);
  }
}

template <class T>
std::vector<T> VectorConditional<T>::readConditionalVector(BufferReader *rdr,
                                                           SaveData *save) {
  VectorConditional<T> vec;
  vec.read(rdr, save);
  return std::move(vec);
}

#endif
