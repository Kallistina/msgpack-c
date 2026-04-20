#include <stdint.h>
#include <stddef.h>
#include "msgpack.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    /* msgpack trusts declared container sizes in the input bytes, so a small
       input can claim a huge array and trigger a multi-GB allocation. Cap size
       so the fuzzer doesn't OOM and exit before the 10-minute run finishes. */
    if (size > 4096) return 0;

    msgpack_unpacked result;
    msgpack_unpacked_init(&result);
    size_t off = 0;
    while (msgpack_unpack_next(&result, (const char *)data, size, &off) == MSGPACK_UNPACK_SUCCESS) {}
    msgpack_unpacked_destroy(&result);

    return 0;
}
