#include "utils.hpp"

int strlen_file(FILE *f) {
    if (f == nullptr)
        return -1;
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, pos, SEEK_SET);
    int count = -1;
    char byte = 0;
    do {
        fread(&byte, 1, 1, f);
        count += 1;
    } while (byte != 0 && ftell(f) != len);
    fseek(f, pos, SEEK_SET);
    if (byte != 0) {
        return -1;
    }
    return count;
}