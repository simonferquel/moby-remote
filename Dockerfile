FROM alpine:3.4
RUN apk --update add make binutils libc-dev libstdc++ g++ cmake clang
WORKDIR /sources
COPY . ./
WORKDIR /build
RUN cmake -DCMAKE_INSTALL_PREFIX=/build/install -DCMAKE_BUILD_TYPE=Release /sources
RUN cmake --build .
RUN ctest -vv
RUN cmake --build . -- install