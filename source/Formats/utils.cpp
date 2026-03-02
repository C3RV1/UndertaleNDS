#include "Formats/utils.hpp"
#include "Engine/Audio.hpp"
#include <cstdio>

int enterFileSection() {
  int v = REG_IE & IRQ_TIMER(Audio2::kTimerIrq);
  irqDisable(IRQ_TIMER(Audio2::kTimerIrq));
  return v;
}

void exitFileSection(int old) { irqEnable(old); }

int str_len_file(FILE *f, char terminator) {
  if (f == nullptr)
    return -1;
  int oldIRQ = enterFileSection();
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
  exitFileSection(oldIRQ);
  return count;
}
