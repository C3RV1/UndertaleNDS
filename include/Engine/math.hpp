//
// Created by cervi on 27/08/2022.
//

#ifndef LAYTON_MATH_HPP
#define LAYTON_MATH_HPP

#include <stdint.h>
#include <math.h>

inline uint32_t double_to_fixed(double input, uint8_t bits)
{
    return (uint32_t)(round(input * (1 << bits)));
}

bool collidesRect(int x1, int y1, int w1, int h1,
                  int x2, int y2, int w2, int h2);

int mod(int a, int b);

#endif //LAYTON_MATH_HPP
