#ifndef TICTACTOE_SERVER_LOGGER_H
#define TICTACTOE_SERVER_LOGGER_H

#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>

template<typename... Args>
void log(Args&&... args) {
    if (sizeof...(args) < 1) {
        return;
    }
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);

    std::clog << "[cpp-server] [" << std::put_time(ptm, "%a %b %d %T %Z %Y") << "] ";
    ((std::clog << args), ...);
    std::clog << std::endl;
}

#endif //TICTACTOE_SERVER_LOGGER_H
