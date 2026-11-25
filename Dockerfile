FROM ubuntu:24.04

RUN apt-get update
RUN apt-get install -y build-essential cmake
RUN rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DARCADE_RESCUE_WITH_RAYLIB=OFF

RUN cmake --build build

RUN ./build/arcade_rescue --headless --ticks 300 --seed 7

CMD ["./build/arcade_rescue", "--headless", "--ticks", "300", "--seed", "7"]
