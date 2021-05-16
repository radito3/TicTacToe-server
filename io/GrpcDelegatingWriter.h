#ifndef TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H
#define TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H

#include <io/DisplayWriter.h>
#include <string_view>
#include "../DisplayWriterService.pb.h"
#include <grpcpp/grpcpp.h>
#include "../grpc/DisplayWorkerClient.h"

class GrpcDelegatingWriter : public DisplayWriterInterface {
    DisplayWorkerClient client;

public:
    explicit GrpcDelegatingWriter(std::string client_address) 
        : client(grpc::CreateChannel(std::move(client_address), grpc::InsecureChannelCredentials())){}

    void write_grid() const override {
        client.write_grid();
    }

    void clear_cell_at(const coordinate &coordinate) const override {
        client.clear_cell_at(coordinate);
    }

    void write_symbol(symbol symbol, const coordinate &coordinate) const override {
        client.write_symbol(symbol, coordinate);
    }

    void write_placeholder_for(symbol symbol, const coordinate &coordinate) const override {
        client.write_placeholder_for(symbol, coordinate);
    }

    void write_stroke(const coordinate &coordinate, stroke_direction direction) const override {
        client.write_stroke(coordinate, direction);
    }

    void write_msg(const std::string_view &msg) const override {
        client.write_msg(msg);
    }

    void write_temp_msg(const std::string_view &msg) const override {
        client.write_temp_msg(msg);
    }

    void flash_placeholder(symbol symbol, const coordinate &coordinate) const override {
        client.flash_placeholder(symbol, coordinate);
    }
};

#endif //TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H
