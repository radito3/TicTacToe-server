cmake_minimum_required(VERSION 3.10)
project(TicTacToe_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(TicTacToe_server main.cpp HttpServer.cpp HttpServer.h request_matcher.h http_methods.h http_response.h http_request.h http_request_context.h thread_pool.h)
