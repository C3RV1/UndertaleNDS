//
// Created by cervi on 27/08/2022.
//

#include "math.hpp"

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

bool collidesRect(int x1, int y1, int w1, int h1,
                  int x2, int y2, int w2, int h2) {
    return (
            x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2
    );
}

int distSquared(int x1, int y1, int x2, int y2) {
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
}

int distSquared_fp(s32 x1, s32 y1, s32 x2, s32 y2) {
    // fixed point w/ 8 bits decimal
    return ((x1-x2)*(x1-x2) >> 8) + ((y1-y2)*(y1-y2) >> 8);
}