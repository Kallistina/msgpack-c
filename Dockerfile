FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    clang \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# cmake configure generates sysdep.h and pack_template.h, then copy them where the source expects them
RUN mkdir -p /src/build && cd /src/build && cmake -DCMAKE_C_COMPILER=clang .. && \
    cp /src/build/include/msgpack/sysdep.h /src/include/msgpack/sysdep.h && \
    cp /src/build/include/msgpack/pack_template.h /src/include/msgpack/pack_template.h

# Compile all msgpack-c sources + fuzz target into one static binary
RUN clang++ -std=c++11 \
    -fsanitize=fuzzer,address -g \
    -I/src/include \
    /src/fuzz/fuzz_target.cpp \
    /src/src/objectc.c \
    /src/src/unpack.c \
    /src/src/version.c \
    /src/src/vrefbuffer.c \
    /src/src/zone.c \
    -o /fuzz_target

ENV ASAN_OPTIONS=allocator_may_return_null=1

CMD ["/bin/sh", "-c", "mkdir -p /fuzzing-output && /fuzz_target -max_total_time=600 -max_len=4096 -artifact_prefix=/fuzzing-output/ /fuzzing-output/ 2>&1 | tee /fuzzing-output/fuzz.log; exit 0"]
