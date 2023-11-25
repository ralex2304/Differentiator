#ifndef CUSTOM_STRINGS_H_
#define CUSTOM_STRINGS_H_

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Gets line from stream
 *
 * @param str
 * @param cnt
 * @param stream
 * @return ssize_t Number of characters were read. -1 in case of error
 */
ssize_t cus_getline(char** str, size_t* cnt, FILE* stream);

/**
 * @brief Geometric realloc
 *
 * @param mem array pointer
 * @param elem_size type size
 * @param size array len
 * @return true if succeed
 * @return false if realloc failed
 */
bool geom_realloc(void** mem, const size_t elem_size, size_t* size);


#endif // #ifndef CUSTOM_STRINGS_H_
