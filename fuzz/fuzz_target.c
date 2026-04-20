#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "msgpack.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0;

    /* 0xdd (array32) and 0xdf (map32) can claim billions of elements and cause OOM */
    if (memchr(data, 0xdd, size) || memchr(data, 0xdf, size)) return 0;

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    size_t offset = 0;
    while (msgpack_unpack_next(&msg, (const char *)data, size, &offset) == MSGPACK_UNPACK_SUCCESS) {}
    msgpack_unpacked_destroy(&msg);

    return 0;
}
