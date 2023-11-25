#include "cus_str.h"

ssize_t cus_getline(char** str, size_t* cnt, FILE* stream) {
    assert(str);
    assert(!*str || *cnt); // check for *cnt > 0 if *str != nullptr
    assert(stream);

    static const size_t BEGIN_ALLOC_SIZE = 256;

    if (*str == nullptr) {
        *cnt = BEGIN_ALLOC_SIZE;
        *str = (char*)calloc(*cnt, sizeof(char));

        if (*str == nullptr)
            return -1;
    }

    size_t i = 0;
    int c = '\0';
    for (i = 0; c != '\n'; i++) {
        if (i >= *cnt - 1)
            if (!geom_realloc((void**)str, sizeof(char), cnt))
                return -1;

        c = getc(stream);
        if (c == EOF)
            return -1;
        (*str)[i] = (char)c;
    }
    (*str)[i] = '\0';

    return i;
}

bool geom_realloc(void** mem, size_t elem_size, size_t* size) {
    *size *= 2;
    void* ptr = *mem;
    *mem = realloc(*mem, (*size) * elem_size);
    if (*mem == nullptr) {
        free(ptr);
        return false;
    }
    return true;
}
