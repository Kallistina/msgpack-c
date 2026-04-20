#include <stdint.h>
#include <stddef.h>
#include "msgpack.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0;

    msgpack_unpacked result;
    msgpack_unpacked_init(&result);
    size_t off = 0;
    while (msgpack_unpack_next(&result, (const char *)data, size, &off) == MSGPACK_UNPACK_SUCCESS) {}
    msgpack_unpacked_destroy(&result);

    return 0;
}
