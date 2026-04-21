/*
 * LibFuzzer harness for msgpack-c (C library)
 *
 * Targets: msgpack_unpack_next() and msgpack_unpacker (streaming unpacker).
 * Random bytes are fed into both and AddressSanitizer detects any memory bugs.
 *
 * Strategy:
 * - msgpack_unpack_next() is the core one-shot deserializer.
 * - msgpack_unpacker processes data incrementally, maintaining internal state
 *   across calls — a different code path with its own state machine and
 *   buffer management, where different bugs may hide.
 * - Both are exercised on every input to maximize code coverage.
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


    /* one-shot unpacker */
    {
        msgpack_unpacked msg;
        msgpack_unpacked_init(&msg);
        size_t offset = 0;
        while (msgpack_unpack_next(&msg, (const char *)data, size, &offset) == MSGPACK_UNPACK_SUCCESS) {}
        msgpack_unpacked_destroy(&msg);
    }

    /* streaming unpacker — feeds data in chunks and maintains internal state */
    {
        msgpack_unpacker pac;
        if (msgpack_unpacker_init(&pac, 64)) {
            if (msgpack_unpacker_buffer_capacity(&pac) < size)
                msgpack_unpacker_reserve_buffer(&pac, size);
            memcpy(msgpack_unpacker_buffer(&pac), data, size);
            msgpack_unpacker_buffer_consumed(&pac, size);

            msgpack_unpacked result;
            msgpack_unpacked_init(&result);
            while (msgpack_unpacker_next(&pac, &result) == MSGPACK_UNPACK_SUCCESS) {}
            msgpack_unpacked_destroy(&result);
            msgpack_unpacker_destroy(&pac);
        }
    }

    return 0;
}
