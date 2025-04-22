//
// Created by cervi on 27/08/2022.
//

#include "Engine/math.hpp"

bool collidesRect(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                  int h2) {
  return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

bool rectContainsOther(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                       int h2) {
  return (x2 >= x1 && y2 >= y1 && x2 + w2 <= x1 + w1 && y2 + h2 <= y1 + h1);
}

int distSquared(int x1, int y1, int x2, int y2) {
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

u32 distSquared_fp(s32 x1, s32 y1, s32 x2, s32 y2) {
  // fixed point w/ 8 bits decimal
  return ((x1 - x2) * (x1 - x2) >> 8) + ((y1 - y2) * (y1 - y2) >> 8);
}

int smoothstep(int a, int b, int t, int maxT) {
  // 3x^2 - 2x^3 if 0 < x < 1
  // 3(t/mt)^2 - 2(t/mt)^3
  // (3t^2*mt - 2t^3)/mt^3
  int coef1 = 3 * t * t * maxT;
  int coef2 = 2 * t * t * t;
  int denom = maxT * maxT * maxT;
  return a + ((b - a) * (coef1 - coef2)) / denom;
}

int easeOutCubic(int a, int b, int counter, int maxCounter) {
  // Ease-out cubic: 1-(1-x)^3
  //                 1-(1-c/m)^3
  //                 1-((m-c)^3/m^3)
  //                 (m^3-(m-c)^3)/m^3
  int maxCubed = maxCounter * maxCounter * maxCounter;
  int diff = (maxCounter - counter);
  int diffCubed = diff * diff * diff;
  int easeOutNominator = maxCubed - diffCubed;

  return a + ((b - a) * easeOutNominator) / maxCubed;
}
