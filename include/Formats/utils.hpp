//
// Created by cervi on 27/08/2022.
//

#ifndef UNDERTALE_UTILS_HPP
#define UNDERTALE_UTILS_HPP

#include <cstdio>

int str_len_file(FILE *f, char terminator);

int enterFileSection();
void exitFileSection(int old);

#endif // UNDERTALE_UTILS_HPP
