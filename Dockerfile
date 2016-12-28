FROM alpine:3.4
RUN apk --update add make binutils libc-dev libstdc++ g++ cmake clang
RUN apk add gdb
RUN echo "@edge http://nl.alpinelinux.org/alpine/edge/main" >> /etc/apk/repositories
RUN apk --update add libexecinfo-dev@edge
WORKDIR /sources
COPY . ./
WORKDIR /build
RUN cmake --config Debug /sources
#RUN make