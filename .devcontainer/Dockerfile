FROM ubuntu:22.04

RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y \
    gcc \
    gdb \
    make \
    libglib2.0-dev \
    bats \
    git \
    pkg-config \
    build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /workspaces/SO

