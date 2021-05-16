#ifndef TICTACTOE_SERVER_INPUTREADERCLIENT_H
#define TICTACTOE_SERVER_INPUTREADERCLIENT_H

#include <grpcpp/grpcpp.h>
#include "../InputReaderService.grpc.pb.h"
#include <io/InputReader.h>
#include <memory>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class InputReaderClient {
	std::unique_ptr<InputReader::Stub> stub;

public:
		explicit InputReaderClient(std::shared_ptr<Channel> channel) : stub(InputReader::NewStub(channel)){}

		input_t read() const {
			Input input;
			ClientContext context;
			google::protobuf::Empty e;
			Status status = stub->read(&context, e, &input);

			if (status.ok()) {
				switch (input.type()) {
				case InputType::DIRECTIONAL:
					return input_t(input.move_dir());
				case InputType::POSITIONAL: {
					auto coord = input.coord();
					return input_t({coord.x(), coord.y()});
				}
				case InputType::SET:
					return input_t(input.set());
				}
				return input_t(move_direction::INVALID);
			}	else {
				std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
				return input_t(move_direction::INVALID);
			}
		}
};

#endif
