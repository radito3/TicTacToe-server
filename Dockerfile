FROM alpine:latest AS build
RUN apk update && apk add --no-cache bash gcc g++ cmake make libstdc++ libgcc musl
WORKDIR /build
COPY . ./
RUN cmake . && make

FROM alpine:latest
WORKDIR /server
COPY --from=build /build/TicTacToe_server ./
EXPOSE 80
CMD ["./TicTacToe_server"]
