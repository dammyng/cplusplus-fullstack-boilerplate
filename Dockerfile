FROM ubuntu:22.04 AS builder

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libboost-all-dev \
    libpqxx-dev \
    libssl-dev \
    inotify-tools

RUN apt-get update && apt-get install -y libpqxx-dev libpq-dev

RUN git clone https://github.com/gabime/spdlog.git /spdlog

RUN mkdir /spdlog/build && \
    cd /spdlog/build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make -j$(nproc) && \
    make install

RUN apt-get install -y nlohmann-json3-dev libspdlog-dev

ENV VCPKG_ROOT=/vcpkg
ENV PATH=/vcpkg:$PATH

# Set the working directory
WORKDIR /app

COPY . .

# Build the application
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Stage 2: Create the runtime image
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libboost-system1.74.0 \
    libboost-program-options1.74.0 \
    libpq5 \
    libpqxx-dev \
    libssl-dev

WORKDIR /app

COPY --from=builder /app/build/cap_returns /app/cap_returns
COPY www/dist /app/www
COPY data /app/data
COPY data /data

EXPOSE 8080

CMD ["./cap_returns", "--doc_root", "/app/www", "--port", "8080"]