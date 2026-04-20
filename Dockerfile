FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    clang \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . /src

# Compile msgpack-c sources + fuzz target together in one step — no shared library needed
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

CMD ["/bin/sh", "-c", "mkdir -p /fuzzing-output && /fuzz_target -max_total_time=600 -artifact_prefix=/fuzzing-output/ /fuzzing-output/ 2>&1 | tee /fuzzing-output/fuzz.log; exit 0"]
