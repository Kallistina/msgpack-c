#include <stdint.h>
#include <stddef.h>
#include "msgpack.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0;

    /* array32 (0xdd) and map32 (0xdf) can claim up to 4 billion elements.
       msgpack's zone allocator grows exponentially trying to fit them, pushing
       RSS into the gigabytes before failing. Skip any input containing them. */
    for (size_t i = 0; i < size; i++) {
        if (data[i] == 0xdd || data[i] == 0xdf) return 0;
    }

    msgpack_unpacked result;
    msgpack_unpacked_init(&result);
    size_t off = 0;
    while (msgpack_unpack_next(&result, (const char *)data, size, &off) == MSGPACK_UNPACK_SUCCESS) {}
    msgpack_unpacked_destroy(&result);

    return 0;
}
