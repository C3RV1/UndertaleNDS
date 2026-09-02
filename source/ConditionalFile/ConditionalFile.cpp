#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"
#include "Cutscene/CutsceneEnums.hpp"
#include <string>

ConditionalReader::ConditionalReader(std::unique_ptr<BufferReader> rdr_) : rdr(std::move(rdr_)) {}

bool ConditionalReader::nextIsConditional() {
  debug_conditional_file("NextIsConditional (last_unc) " +
                       std::to_string(_lastUnconditionalCount));
  if (_lastUnconditionalCount == 0) {
    u8 v;
    rdr->read(&v, 1);
    debug_conditional_file("Needed one more value: read " + std::to_string(v));

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

Condition readCondition(BufferReader* rdr) {
  u16 flag, cmp_value;
  u8 cmp;
  rdr->read(&flag, 2);
  rdr->read(&cmp, 1);
  rdr->read(&cmp_value, 2);
  return Condition(flag, cmp, cmp_value);
}

u8 readValue(tag<u8>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading u8 value");
  u8 data;
  cr->rdr->read(&data, 1);
  return data;
}

u16 readValue(tag<u16>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading u16 value");
  u16 data;
  cr->rdr->read(&data, 2);
  return data;
}

u32 readValue(tag<u32>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading u32 value");
  u32 data;
  cr->rdr->read(&data, 4);
  return data;
}

s8 readValue(tag<s8>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading s8 value");
  s8 data;
  cr->rdr->read(&data, 1);
  return data;
}

s16 readValue(tag<s16>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading s16 value");
  s16 data;
  cr->rdr->read(&data, 2);
  return data;
}

s32 readValue(tag<s32>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading s32 value");
  s32 data;
  cr->rdr->read(&data, 4);
  return data;
}

bool readValue(tag<bool>, ConditionalReader *cr, SaveData *save) {
  debug_conditional_file("Reading bool value");
  bool data;
  cr->rdr->read(&data, 1);
  return data;
}

std::string readValue(tag<std::string>, ConditionalReader *cr,
                                SaveData *save) {
  debug_conditional_file("Reading string value");
  return cr->rdr->readstring();
}

