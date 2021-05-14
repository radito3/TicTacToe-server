#ifndef TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H
#define TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H

#include <io/DisplayWriter.h>
#include <string_view>

class GrpcDelegatingWriter : public DisplayWriter {

public:

    void write_grid() const override {

    }

    void clear_cell_at(const Coordinate &coordinate) const override {

    }

    void write_symbol(Symbol symbol, const Coordinate &coordinate) const override {

    }

    void write_placeholder_for(Symbol symbol, const Coordinate &coordinate) const override {

    }

    void write_stroke(const Coordinate &coordinate, StrokeDirection direction) const override {

    }

    void write_msg(const std::string_view &view) const override {

    }

    void write_temp_msg(const std::string_view &view) const override {

    }

    void flash_placeholder(Symbol symbol, const Coordinate &coordinate) const override {

    }
};

#endif //TICTACTOE_SERVER_GRPCDELEGATINGWRITER_H
