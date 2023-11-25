#ifndef MACROS_H_
#define MACROS_H_

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#ifdef __TXLIB_H_INCLUDED
#undef MAX
#undef MIN
#endif //< #ifdef __TXLIB_H_INCLUDED

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FREE(ptr) do {                \
                      free(ptr);      \
                      ptr = nullptr;  \
                  } while (0)

inline void* recalloc(void* ptr, size_t old_size, size_t new_size) {
    ptr = realloc(ptr, new_size);

    if (!ptr)
        return ptr;

    if (new_size > old_size)
        memset((char*)ptr + old_size, 0, new_size - old_size);

    return ptr;
}

inline void fill(void* array, size_t len, const void* elem, const size_t elem_size) {
    assert(array);
    assert(elem);

    while (len--) {
        memcpy(array, elem, elem_size);
        array = (char*)array + elem_size;
    }
}

struct VarCodeData {
    const char* name = "Not specified";
    const char* file = "Not specified";
          int   line = -1;
    const char* func = "Not specified";
};

#define VAR_CODE_DATA(name) {# name, __FILE__, __LINE__, __func__}
#define VAR_CODE_DATA_PTR(name) {(# name) + 1, __FILE__, __LINE__, __func__}

#ifndef unix

inline char* strndup(const char* src, const size_t n) {
    assert(src);

    size_t src_len = strlen(src);

    char* dest = (char*)calloc(n + 1, sizeof(char));

    if (!dest) return nullptr;

    strncpy(dest, src, MIN(n, src_len));
    dest[MIN(n, src_len)] = '\0';

    return dest;
}

#endif //< #ifndef unix

#endif // #ifndef MACROS_H_
