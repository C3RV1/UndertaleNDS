//
// Created by cervi on 27/08/26
//
#ifndef UNDERTALE_CONDITIONAL_FILE_HPP
#define UNDERTALE_CONDITIONAL_FILE_HPP

#include "DEBUG_FLAGS.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"
#include <nds.h>
#include <optional>
#include <string>

class ConditionalReader {
public:
  ConditionalReader(std::unique_ptr<BufferReader> rdr_);
  bool nextIsConditional();
  std::unique_ptr<BufferReader> rdr;

private:
  u8 _lastUnconditionalCount = 0;
};

class ConditionalObj {
public:
  virtual void read(ConditionalReader *cr, SaveData *save) = 0;
};

class Condition {
public:
  Condition(u16 flag, u8 cmp, u16 cmp_value);
  bool checkCondition(SaveData *save);
  bool hasMoreConditions();
  bool hasMoreVariations();
  bool orWithPrevious();

  std::string to_string();

private:
  static constexpr u8 kFlipBit = 1 << 2;
  static constexpr u8 kHasNextConditionBit = 1 << 3;
  static constexpr u8 kHasNextVariationBit = 1 << 4;
  static constexpr u8 kOrWithPreviousBit = 1 << 5;

  u16 _flag, _cmp_value;
  u8 _cmp;
};

Condition readCondition(BufferReader *rdr);

template <typename> struct tag {};

u8 readValue(tag<u8>, ConditionalReader *cr, SaveData *save);
u16 readValue(tag<u16>, ConditionalReader *cr, SaveData *save);
u32 readValue(tag<u32>, ConditionalReader *cr, SaveData *save);
s8 readValue(tag<s8>, ConditionalReader *cr, SaveData *save);
s16 readValue(tag<s16>, ConditionalReader *cr, SaveData *save);
s32 readValue(tag<s32>, ConditionalReader *cr, SaveData *save);
bool readValue(tag<bool>, ConditionalReader *cr, SaveData *save);
std::string readValue(tag<std::string>, ConditionalReader *cr,
                                 SaveData *save);

#if defined(DEBUG_CONDITIONAL_FILE) && !defined(__GXX_RTTI)
#error "Debugging ConditionalFile requires RTTI enabled. Remove -fno-rtti."
#endif

#ifdef DEBUG_CONDITIONAL_FILE
template <typename T>
struct TypeName {
  static std::string Get() {
    return typeid(T).name();
  }
};
#endif

template <class T>
T readConditionalData(ConditionalReader *cr, SaveData *save, ConditionalObj *obj) {
#ifdef DEBUG_CONDITIONAL_FILE
  debug_conditional_file("readConditionalData<" + TypeName<T>::Get() + ">");
#endif

  if (!cr->nextIsConditional()) {
    debug_conditional_file("Unconditional data");
    return readValue(tag<T>{}, cr, save);
  }
  debug_conditional_file("Conditional data");

  std::optional<T> data = {};
  std::optional<Condition> c = {};
  do {
    bool areConditionsTrue = true;
    do {
      c = readCondition(cr->rdr.get());
      debug_conditional_file("Condition: " + c->to_string() + " evaluates to " +
                           std::to_string(c->checkCondition(save)));
      if (!c->orWithPrevious())
        areConditionsTrue &= c->checkCondition(save);
      else
        areConditionsTrue |= c->checkCondition(save);
    } while (c->hasMoreConditions());

    if (areConditionsTrue && !data) {
      debug_conditional_file("Conditions passed! Data set");
      data = readValue(tag<T>{}, cr, save);
    } else {
      debug_conditional_file("Conditions did not pass... Discarding");
      T _ = readValue(tag<T>{}, cr, save);
    }
  } while (c->hasMoreVariations());

  if (!data) {
    debug_conditional_file("Returning default value");
    return readValue(tag<T>{}, cr, save);
  } else {
    debug_conditional_file("Discarding default value");
    T _ = readValue(tag<T>{}, cr, save);
  }

  return *data;
}

template <class T>
class VectorConditional : public std::vector<T>, public ConditionalObj {
public:
  void read(ConditionalReader *cr, SaveData *save) override;
  static std::vector<T> readConditionalVector(ConditionalReader *cr,
                                              SaveData *save);
};

template <class T>
void VectorConditional<T>::read(ConditionalReader *cr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  debug_conditional_file("VectorCondition::read<" + TypeName<T>::Get() + ">");
#endif
  u16 num_elements;
  cr->rdr->read(&num_elements, 2);
  for (size_t i = 0; i < num_elements; i++) {
    if (!cr->nextIsConditional()) {
      debug_conditional_file("Unconditional data");
      this->push_back(readValue(tag<T>{}, cr, save));
      continue;
    }

    debug_conditional_file("Conditional data");
    std::optional<Condition> c = {};
    bool areConditionsTrue = true;
    do {
      c = readCondition(cr->rdr.get());
      debug_conditional_file("Condition: " + c->to_string() + " evaluates to " +
                           std::to_string(c->checkCondition(save)));
      if (!c->orWithPrevious())
        areConditionsTrue &= c->checkCondition(save);
      else
        areConditionsTrue |= c->checkCondition(save);
    } while (c->hasMoreConditions());

    u8 value_count;
    cr->rdr->read(&value_count, 1);
    debug_conditional_file("Condition has " + std::to_string(value_count) +
                         " values");

    for (int j = 0; j < value_count; j++) {
      T data = readValue(tag<T>{}, cr, save);
      if (areConditionsTrue) {
        debug_conditional_file("Conditions passed! Pushing back");
        this->push_back(data);
      }
      else
        debug_conditional_file("Conditions did not pass... Discarding");
    }

  }
}

template <class T>
std::vector<T> VectorConditional<T>::readConditionalVector(ConditionalReader *cr,
                                                           SaveData *save) {
  VectorConditional<T> vec;
  vec.read(cr, save);
  return std::move(vec);
}

#endif
