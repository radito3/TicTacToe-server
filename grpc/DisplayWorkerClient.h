#ifndef TICTACTOE_SERVER_DISPLAYWRITERCLIENT_H
#define TICTACTOE_SERVER_DISPLAYWRITERCLIENT_H

#include <grpcpp/grpcpp.h>
#include <io/DisplayWriter.h>
#include "../DisplayWriterService.grpc.pb.h"
#include <memory>
#include <string_view>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

std::ostream &operator<<(std::ostream &out, const Status &status)
{
	out << status.error_code() << ": " << status.error_message() << std::endl;
	return out;
};

class DisplayWorkerClient
{
	std::unique_ptr<DisplayWriter::Stub> stub;
	google::protobuf::Empty empty_request;

public:
	explicit DisplayWorkerClient(std::shared_ptr<Channel> channel) : stub(DisplayWriter::NewStub(channel)) {}

	void write_grid() const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		Status status = stub->write_grid(&context, empty_request, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void clear_cell_at(const coordinate &coordinate) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		Coordinate coord;
		coord.set_x(coordinate.x);
		coord.set_y(coordinate.y);
		Status status = stub->clear_cell_at(&context, coord, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void write_symbol(symbol symbol, const coordinate &coordinate) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		WriteIconMessage write_icon_msg;
		write_icon_msg.set_symbol(Symbol(static_cast<int>(symbol)));
		Coordinate coord;
		coord.set_x(coordinate.x);
		coord.set_y(coordinate.y);
		write_icon_msg.set_allocated_coord(&coord);

		Status status = stub->write_symbol(&context, write_icon_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void write_placeholder_for(symbol symbol, const coordinate &coordinate) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		WriteIconMessage write_icon_msg;
		write_icon_msg.set_symbol(Symbol(static_cast<int>(symbol)));
		Coordinate coord;
		coord.set_x(coordinate.x);
		coord.set_y(coordinate.y);
		write_icon_msg.set_allocated_coord(&coord);

		Status status = stub->write_placeholder_for(&context, write_icon_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void write_stroke(const coordinate &coordinate, stroke_direction direction) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		WriteStrokeMessage write_stroke_msg;
		write_stroke_msg.set_stroke_dir(StrokeDirection(static_cast<int>(direction)));
		Coordinate coord;
		coord.set_x(coordinate.x);
		coord.set_y(coordinate.y);
		write_stroke_msg.set_allocated_coord(&coord);

		Status status = stub->write_stroke(&context, write_stroke_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void write_msg(const std::string_view &msg) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		TextMessage text_msg;
		text_msg.set_value(msg.data());

		Status status = stub->write_msg(&context, text_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void write_temp_msg(const std::string_view &msg) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		TextMessage text_msg;
		text_msg.set_value(msg.data());

		Status status = stub->write_temp_msg(&context, text_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}

	void flash_placeholder(symbol symbol, const coordinate &coordinate) const
	{
		ClientContext context;
		google::protobuf::Empty empty_response;
		WriteIconMessage write_icon_msg;
		write_icon_msg.set_symbol(Symbol(static_cast<int>(symbol)));
		Coordinate coord;
		coord.set_x(coordinate.x);
		coord.set_y(coordinate.y);
		write_icon_msg.set_allocated_coord(&coord);

		Status status = stub->flash_placeholder(&context, write_icon_msg, &empty_response);

		if (!status.ok())
		{
			std::cerr << status;
		}
	}
};

#endif
