cmake_minimum_required(VERSION 3.16)
project(HP_HTTP_Server VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hp_http_core
    src/base/logger.cpp
    src/net/inet_address.cpp
    src/net/socket.cpp
    src/net/epoller.cpp
    src/net/tcp_server.cpp
    src/http/http_request.cpp
    src/http/http_response.cpp
    src/http/http_server.cpp
)

target_include_directories(hp_http_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # PUBLIC 让 hp_http_core 自己和链接它的目标都传递了 include 路径
target_compile_options(hp_http_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(hp_http_server app/main.cpp)
target_link_libraries(hp_http_server PRIVATE hp_http_core) # 链接 hp_http_core 的目标就是 hp_http_server
target_compile_options(hp_http_server PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
add_subdirectory(tests)
