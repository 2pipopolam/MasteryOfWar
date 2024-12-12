FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    libjsoncpp-dev \
    libsqlite3-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir -p DB
COPY DB/game.db DB/

RUN mkdir -p build
WORKDIR /app/build

RUN cmake .. && \
    cmake --build . --config Release

EXPOSE 7777

CMD ["./GameServer"]