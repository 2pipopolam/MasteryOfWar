FROM ubuntu:22.04

# Prevent timezone prompt during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libboost-all-dev \
    libjsoncpp-dev \
    libsqlite3-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Create and enter build directory
RUN mkdir -p build
WORKDIR /app/build

# Configure and build
RUN cmake .. && \
    cmake --build . --config Release

# Expose the game server port (adjust if needed)
EXPOSE 7777

# Set the entry point
CMD ["./GameServer"]