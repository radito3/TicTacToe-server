#ifndef TICTACTOE_SERVER_GRPCDELEGATINGREADER_H
#define TICTACTOE_SERVER_GRPCDELEGATINGREADER_H

#include <io/InputReader.h>

class GrpcDelegatingReader : public InputReader {
//    InputReaderClient client;

public:
//    GrpcDelegatingReader(InputReaderClient) {}

    input_t read() const override {
        return input_t(true);
    }
};

#endif //TICTACTOE_SERVER_GRPCDELEGATINGREADER_H
