#include <cstdint>
#include <cstddef>
#include <cstdlib>

#include "msgpack.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size > 4096) return 0;
    // Fuzz the one-shot unpacker
    {
        msgpack_unpacked result;
        msgpack_unpacked_init(&result);
        size_t off = 0;
        msgpack_unpack_return ret;
        do {
            ret = msgpack_unpack_next(&result, (const char *)data, size, &off);
        } while (ret == MSGPACK_UNPACK_SUCCESS);
        msgpack_unpacked_destroy(&result);
    }

    // Fuzz the streaming unpacker
    {
        msgpack_unpacker pac;
        if (msgpack_unpacker_init(&pac, 64)) {
            if (msgpack_unpacker_buffer_capacity(&pac) < size) {
                msgpack_unpacker_reserve_buffer(&pac, size);
            }
            memcpy(msgpack_unpacker_buffer(&pac), data, size);
            msgpack_unpacker_buffer_consumed(&pac, size);

            msgpack_unpacked result;
            msgpack_unpacked_init(&result);
            while (msgpack_unpacker_next(&pac, &result) == MSGPACK_UNPACK_SUCCESS) {
                // consume all objects
            }
            msgpack_unpacked_destroy(&result);
            msgpack_unpacker_destroy(&pac);
        }
    }

    return 0;
}
