//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_MATH_HPP
#define UNDERTALE_MATH_HPP

#include <cmath>
#include <nds.h>
#include <utility>

inline u32 double_to_fixed(double input, u8 bits) {
  return (u32)(round(input * (1 << bits)));
}

bool collidesRect(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                  int h2);
bool rectContainsOther(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
                       int h2);

int distSquared(int x1, int y1, int x2, int y2);
u32 distSquared_fp(s32 x1, s32 y1, s32 x2,
                   s32 y2); // fixed point distance squared

inline int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

inline int lerp(int a, int b, int t, int maxT) {
  return a + ((b - a) * t / maxT);
}

inline std::pair<s32, s32> lerp(std::pair<s32, s32> &a, std::pair<s32, s32> &b,
                                int t, int maxT) {
  return {lerp(a.first, b.first, t, maxT), lerp(a.second, b.second, t, maxT)};
}

int easeOutCubic(int a, int b, int counter, int maxCounter);

int smoothstep(int a, int b, int t, int maxT);

#endif // UNDERTALE_MATH_HPP
