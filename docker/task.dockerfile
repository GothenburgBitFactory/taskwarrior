FROM ubuntu:22.04 AS base

FROM base AS builder

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install -y \
            build-essential \
            cmake \
            curl \
            git \
            libgnutls28-dev \
            uuid-dev

# Setup language environment
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

# Add source directory
ADD .. /root/code/
WORKDIR /root/code/

# Setup Rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs > rustup.sh && \
    sh rustup.sh -y --profile minimal --default-toolchain stable --component rust-docs

# Build Taskwarrior
RUN git clean -dfx && \
    git submodule init && \
    git submodule update && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release . && \
    cmake --build build -j 8

FROM base AS runner

# Install Taskwarrior
COPY --from=builder /root/code/build/src/task /usr/local/bin

# Initialize Taskwarrior
RUN ( echo "yes" | task ) || true
