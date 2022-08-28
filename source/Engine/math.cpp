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