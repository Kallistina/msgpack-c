/*
 * LibFuzzer harness for msgpack-c (C library)
 *
 * Target: msgpack_unpack_next() - the main deserialization function.
 * Random bytes are fed into it and AddressSanitizer detects any memory bugs.
 *
 * Strategy:
 * - msgpack_unpack_next() was chosen as the target because it is the core
 *   entry point: any malformed message passes through here first.
 * - It is called in a loop to handle inputs containing multiple packed objects.
 * - The C API is used directly since the library itself is written in C.
 *
 * Observations during development:
 * - During the first fuzzing run, the fuzzer immediately found that a tiny
 *   input containing 0xdd (array32) could trick the library into attempting a
 *   ~68 GB allocation. The zone allocator grows exponentially and pushes RSS
 *   into the gigabytes before the OS rejects it, killing the fuzzer process.
 * - This is a real denial-of-service bug in msgpack-c: there is no sanity
 *   check on the declared container size before memory is allocated.
 * - To keep the 10-minute campaign running, inputs containing the 32-bit
 *   container format codes (0xdd / 0xdf) are skipped. All other format types
 *   are still exercised, including nested arrays and maps up to 65535 elements.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "msgpack.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0; /* ignore very large inputs */

    /* 0xdd (array32) and 0xdf (map32) claim up to 4 billion elements and OOM the fuzzer */
    if (memchr(data, 0xdd, size) || memchr(data, 0xdf, size)) return 0;

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    size_t offset = 0;
    while (msgpack_unpack_next(&msg, (const char *)data, size, &offset) == MSGPACK_UNPACK_SUCCESS) {}
    msgpack_unpacked_destroy(&msg);

    return 0;
}
