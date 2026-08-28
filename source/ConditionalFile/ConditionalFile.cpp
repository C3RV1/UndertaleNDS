#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include <string>

bool ConditionalObj::nextIsConditional(BufferReader *rdr) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("NextIsConditional (last_unc) " +
               std::to_string(_lastUnconditionalCount));
#endif
  if (_lastUnconditionalCount == 0) {
    u8 v;
    rdr->read(&v, 1);
#ifdef DEBUG_CONDITIONAL_FILE
    Engine::log_("Needed one more value: read " + std::to_string(v));
#endif

    if (v == 0xFF)
      return true;

    _lastUnconditionalCount = v + 1;
  }

  _lastUnconditionalCount--;
  return false;
}

Condition::Condition(u16 flag, u8 cmp, u16 cmp_value)
    : _flag(flag), _cmp_value(cmp_value), _cmp(cmp) {}

bool Condition::checkCondition(SaveData* save) {
  u8 comparator = _cmp & 0b11;
  bool flip = _cmp & kFlipBit;

  bool v;
  
  switch (comparator) {
  case ComparisonOperator::EQUALS:
    v = save->flags[_flag] == _cmp_value;
    break;
  case ComparisonOperator::GREATER_THAN:
    v = save->flags[_flag] > _cmp_value;
    break;
  case ComparisonOperator::LESS_THAN:
    v = save->flags[_flag] < _cmp_value;
    break;
  default:
    Engine::throw_("Invalid room condition comparator " + std::to_string(comparator));
    break;
  }

  if (flip)
    v = !v;

  return v;
}

bool Condition::hasMoreConditions() {
  return _cmp & kHasNextConditionBit;
}

bool Condition::hasMoreVariations() {
  return _cmp & kHasNextVariationBit;
}

bool Condition::orWithPrevious() {
  return _cmp & kOrWithPreviousBit;
}

#ifdef DEBUG_CONDITIONAL_FILE
std::string Condition::to_string() {
  std::string op;
  bool flip = _cmp & kFlipBit;
  switch (_cmp & 0b11) {
  case ComparisonOperator::EQUALS:
    if (!flip)
      op = "==";
    else
      op = "!=";
    break;
  case ComparisonOperator::GREATER_THAN:
    if (!flip)
      op = ">";
    else
      op = "<=";
    break;
  case ComparisonOperator::LESS_THAN:
    if (!flip)
      op = "<";
    else
      op = ">=";
    break;
  default:
    if (!flip)
      op = "INV";
    else
      op = "INVF";
    break;
  }

  return std::to_string(_flag) + " " + op + " " + std::to_string(_cmp_value);
}
#endif

Condition readCondition(BufferReader* rdr) {
  u16 flag, cmp_value;
  u8 cmp;
  rdr->read(&flag, 2);
  rdr->read(&cmp, 1);
  rdr->read(&cmp_value, 2);
  return Condition(flag, cmp, cmp_value);
}

u8 readConditionalValue(tag<u8>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading u8 value");
#endif
  u8 data;
  rdr->read(&data, 1);
  return data;
}

u16 readConditionalValue(tag<u16>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading u16 value");
#endif
  u16 data;
  rdr->read(&data, 2);
  return data;
}

u32 readConditionalValue(tag<u32>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading u32 value");
#endif
  u32 data;
  rdr->read(&data, 4);
  return data;
}

s8 readConditionalValue(tag<s8>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading s8 value");
#endif
  s8 data;
  rdr->read(&data, 1);
  return data;
}

s16 readConditionalValue(tag<s16>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading s16 value");
#endif
  s16 data;
  rdr->read(&data, 2);
  return data;
}

s32 readConditionalValue(tag<s32>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading s32 value");
#endif
  s32 data;
  rdr->read(&data, 4);
  return data;
}

bool readConditionalValue(tag<bool>, BufferReader *rdr, SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading bool value");
#endif
  bool data;
  rdr->read(&data, 1);
  return data;
}

std::string readConditionalValue(tag<std::string>, BufferReader *rdr,
                                SaveData *save) {
#ifdef DEBUG_CONDITIONAL_FILE
  Engine::log_("Reading string value");
#endif
  return rdr->readstring();
}

