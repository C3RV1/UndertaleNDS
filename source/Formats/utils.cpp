#include "Formats/utils.hpp"

int strlen_file(FILE *f, char terminator) {
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
    } while (byte != terminator && ftell(f) != len);
    fseek(f, pos, SEEK_SET);
    return count;
}