FROM cpp-build-base:0.1.0 AS build

WORKDIR /src

COPY . ./

RUN cmake . && make

FROM ubuntu:bionic

WORKDIR /opt/hello-world

COPY --from=build /src/TicTacToe_server ./

EXPOSE 80

CMD ["./TicTacToe_server"]

