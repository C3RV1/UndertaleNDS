#include "ConditionalFile/ConditionalFile.hpp"
#include "Engine/Engine.hpp"

bool ConditionalObj::nextIsConditional(BufferReader *rdr) {
  if (_lastUnconditionalCount == 0) {
    u8 v;
    rdr->read(&v, 1);

    if (v == 0xFF)
      return true;

    _lastUnconditionalCount = v + 1;
  }

  _lastUnconditionalCount--;
  return true;
}

Condition::Condition(u16 flag, u8 cmp, u16 cmp_value)
    : _flag(flag), _cmp_value(cmp_value), _cmp(cmp) {}

bool Condition::checkCondition(SaveData* save) {
  u8 comparator = _cmp & 0b11;
  bool flip = _cmp & kFlipBit;

  bool v;
  
  switch (comparator) {
  case 0:
    v = save->flags[_flag] == _cmp_value;
    break;
  case 1:
    v = save->flags[_flag] > _cmp_value;
    break;
  case 2:
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

Condition readCondition(BufferReader* rdr) {
  u16 flag, cmp_value;
  u8 cmp;
  rdr->read(&flag, 2);
  rdr->read(&cmp, 1);
  rdr->read(&cmp_value, 2);
  return Condition(flag, cmp, cmp_value);
}

u8 readConditionalValue(tag<u8>, BufferReader *rdr, SaveData *save) {
  u8 data;
  rdr->read(&data, 1);
  return data;
}

u16 readConditionalValue(tag<u16>, BufferReader *rdr, SaveData *save) {
  u16 data;
  rdr->read(&data, 2);
  return data;
}

u32 readConditionalValue(tag<u32>, BufferReader *rdr, SaveData *save) {
  u32 data;
  rdr->read(&data, 4);
  return data;
}

s8 readConditionalValue(tag<s8>, BufferReader *rdr, SaveData *save) {
  s8 data;
  rdr->read(&data, 1);
  return data;
}

s16 readConditionalValue(tag<s16>, BufferReader *rdr, SaveData *save) {
  s16 data;
  rdr->read(&data, 2);
  return data;
}

s32 readConditionalValue(tag<s32>, BufferReader *rdr, SaveData *save) {
  s32 data;
  rdr->read(&data, 4);
  return data;
}

bool readConditionalValue(tag<bool>, BufferReader *rdr, SaveData *save) {
  bool data;
  rdr->read(&data, 1);
  return data;
}

std::string readConditionalValue(tag<std::string>, BufferReader *rdr,
                                SaveData *save) {
  return rdr->readstring();
}

