#ifndef TICTACTOE_SERVER_GRPCDELEGATINGREADER_H
#define TICTACTOE_SERVER_GRPCDELEGATINGREADER_H

#include <io/InputReader.h>
#include <grpcpp/grpcpp.h>
#include "../grpc/InputReaderClient.h"
#include "../InputReaderService.grpc.pb.h"

class GrpcDelegatingReader : public InputReaderInterface {
    InputReaderClient client;

public:
    explicit GrpcDelegatingReader(std::string client_address) 
        : client(grpc::CreateChannel(std::move(client_address), grpc::InsecureChannelCredentials())) {}

    input_t read() const override {
        return client.read();
    }
};

#endif //TICTACTOE_SERVER_GRPCDELEGATINGREADER_H
