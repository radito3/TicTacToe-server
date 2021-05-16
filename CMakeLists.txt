cmake_minimum_required(VERSION 3.17)
project(TicTacToe_server)

include(FetchContent)

FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG v1.37.1
)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(gRPC)

add_subdirectory(deps/TicTacToe)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

add_executable(TicTacToe_server main.cpp HttpServer.cpp HttpServer.h request_matcher.h http_methods.h http_response.h http_request.h http_request_context.h thread_pool.h io/GrpcDelegatingWriter.h io/GrpcDelegatingReader.h DisplayWriterService.grpc.pb.cc DisplayWriterService.grpc.pb.h DisplayWriterService.pb.cc DisplayWriterService.pb.h InputReaderService.grpc.pb.h InputReaderService.grpc.pb.cc InputReaderService.pb.h InputReaderService.pb.cc Coordinate.grpc.pb.h Coordinate.grpc.pb.cc Coordinate.pb.cc Coordinate.pb.h grpc/DisplayWorkerClient.h grpc/InputReaderClient.h connection.h handle_connection_job.h logger.h "SearchManager.h" "LobbyManager.h" "GameSessionsManager.h")
target_link_libraries(${PROJECT_NAME} TicTacToe grpc++)
