cmake_minimum_required(VERSION 3.17)
project(TicTacToe_server)

add_subdirectory(deps/TicTacToe)

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(TicTacToe_server main.cpp HttpServer.cpp HttpServer.h request_matcher.h http_methods.h http_response.h http_request.h http_request_context.h thread_pool.h io/GrpcDelegatingWriter.h io/GrpcDelegatingReader.h)
target_link_libraries(${PROJECT_NAME} TicTacToe)
