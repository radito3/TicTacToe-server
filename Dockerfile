FROM alpine:latest AS build
RUN apk update && apk add --no-cache bash gcc g++ cmake make git linux-headers
WORKDIR /build
COPY . ./
RUN cmake . && make

FROM alpine:latest
RUN apk add --no-cache libstdc++ libgcc
COPY --from=build /build/TicTacToe_server ./
EXPOSE 80
CMD ["./TicTacToe_server"]
