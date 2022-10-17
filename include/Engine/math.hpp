//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_MATH_HPP
#define UNDERTALE_MATH_HPP

#define ARM9
#include <nds.h>
#include <cmath>

inline u32 double_to_fixed(double input, u8 bits)
{
    return (u32)(round(input * (1 << bits)));
}

bool collidesRect(int x1, int y1, int w1, int h1,
                  int x2, int y2, int w2, int h2);

int distSquared(int x1, int y1, int x2, int y2);
u32 distSquared_fp(s32 x1, s32 y1, s32 x2, s32 y2); // fixed point distance squared

int mod(int a, int b);

#endif //UNDERTALE_MATH_HPP
