//
// Created by cervi on 27/08/26
//
#ifndef UNDERTALE_CONDITIONAL_FILE_HPP
#define UNDERTALE_CONDITIONAL_FILE_HPP

#include "DEBUG_FLAGS.hpp"
#include "Engine/Engine.hpp"
#include "Formats/utils.hpp"
#include "Save.hpp"
#include <nds.h>
#include <optional>
#include <string>

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
  bool orWithPrevious();

#ifdef DEBUG_CONDITIONAL_FILE
  std::string to_string();
#endif

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

u8 readConditionalValue(tag<u8>, BufferReader *rdr, SaveData *save);
u16 readConditionalValue(tag<u16>, BufferReader *rdr, SaveData *save);
u32 readConditionalValue(tag<u32>, BufferReader *rdr, SaveData *save);
s8 readConditionalValue(tag<s8>, BufferReader *rdr, SaveData *save);
s16 readConditionalValue(tag<s16>, BufferReader *rdr, SaveData *save);
s32 readConditionalValue(tag<s32>, BufferReader *rdr, SaveData *save);
bool readConditionalValue(tag<bool>, BufferReader *rdr, SaveData *save);
std::string readConditionalValue(tag<std::string>, BufferReader *rdr,
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
T readConditionalData(BufferReader *rdr, SaveData *save, ConditionalObj *obj) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("readConditionalData<" + TypeName<T>::Get() + ">");
#endif
  if (!obj->nextIsConditional(rdr)) {
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Unconditional data");
#endif
    return readConditionalValue(tag<T>{}, rdr, save);
  }
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Conditional data");
#endif

  std::optional<T> data = {};
  std::optional<Condition> c = {};
  do {
    bool areConditionsTrue = true;
    do {
      c = readCondition(rdr);
#ifdef DEBUG_CONDITIONAL_FILE
      Engine::log_("Condition: " + c->to_string() + " evaluates to " +
                   std::to_string(c->checkCondition(save)));
#endif
      if (!c->orWithPrevious())
        areConditionsTrue &= c->checkCondition(save);
      else
        areConditionsTrue |= c->checkCondition(save);
    } while (c->hasMoreConditions());

    if (areConditionsTrue && !data) {
#ifdef DEBUG_CONDITIONAL_FILE
      Engine::log_("Conditions passed! Data set");
#endif
      data = readConditionalValue(tag<T>{}, rdr, save);
    } else {
#ifdef DEBUG_CONDITIONAL_FILE
      Engine::log_("Conditions did not pass... Discarding");
#endif
      T _ = readConditionalValue(tag<T>{}, rdr, save);
    }
  } while (c->hasMoreVariations());

  if (!data) {
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Returning default value");
#endif
    return readConditionalValue(tag<T>{}, rdr, save);
  } else {
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Discarding default value");
#endif
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
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("VectorCondition::read<" + TypeName<T>::Get() + ">");
#endif
  u16 num_elements;
  rdr->read(&num_elements, 2);
  for (size_t i = 0; i < num_elements; i++) {
    if (!nextIsConditional(rdr)) {
#ifdef DEBUG_CONDITIONAL_FILE
      Engine::log_("Unconditional data");
#endif
      this->push_back(readConditionalValue(tag<T>{}, rdr, save));
      continue;
    }
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Conditional data");
#endif

    std::optional<Condition> c = {};
    bool areConditionsTrue = true;
    do {
      c = readCondition(rdr);
#ifdef DEBUG_CONDITIONAL_FILE
      Engine::log_("Condition: " + c->to_string() + " evaluates to " +
                   std::to_string(c->checkCondition(save)));
#endif
      if (!c->orWithPrevious())
        areConditionsTrue &= c->checkCondition(save);
      else
        areConditionsTrue |= c->checkCondition(save);
    } while (c->hasMoreConditions());

    u8 value_count;
    rdr->read(&value_count, 1);
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Condition has " + std::to_string(value_count) + " values");
#endif

    for (int j = 0; j < value_count; j++) {
      T data = readConditionalValue(tag<T>{}, rdr, save);
      if (areConditionsTrue) {
#ifdef DEBUG_CONDITIONAL_FILE
        Engine::log_("Conditions passed! Pushing back");
#endif
        this->push_back(data);
      }
#ifdef DEBUG_CONDITIONAL_FILE
      else
        Engine::log_("Conditions did not pass... Discarding");
#endif
    }

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
