#include "crc32.h"

CRC32_t crc_add(CRC32_t crc, const char* data, ssize_t data_size) {
    assert(data);

    while (data_size--) {
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *data) & 255];
        data++;
    }

    return crc;
}
