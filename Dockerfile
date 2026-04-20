FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    clang \
    cmake \
    make \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# Build msgpack-c with AddressSanitizer (fuzzer-no-link so library doesn't pull in fuzzer runtime)
RUN mkdir build && cd build && \
    cmake -DCMAKE_C_COMPILER=clang \
          -DCMAKE_C_FLAGS="-fsanitize=address,fuzzer-no-link -g" \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          .. && \
    make -j$(nproc) && \
    make install

# Compile the fuzzer binary, linking in the fuzzer runtime
RUN clang++ -std=c++11 \
    -fsanitize=fuzzer,address -g \
    -I/usr/local/include \
    /src/fuzz/fuzz_target.cpp \
    -L/usr/local/lib \
    -lmsgpack-c \
    -o /fuzz_target

CMD mkdir -p /fuzzing-output && \
    /fuzz_target \
        -max_total_time=600 \
        -artifact_prefix=/fuzzing-output/ \
        /fuzzing-output/ \
        2>&1 | tee /fuzzing-output/fuzz.log; exit 0
