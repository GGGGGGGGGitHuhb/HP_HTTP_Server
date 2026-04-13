add_executable(http_request_test http_request_test.cpp)
target_link_libraries(http_request_test PRIVATE hp_http_core)
target_compile_options(http_request_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME http_request_test COMMAND http_request_test)

add_executable(http_response_test http_response_test.cpp)
target_link_libraries(http_response_test PRIVATE hp_http_core)
target_compile_options(http_response_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME http_response_test COMMAND http_response_test)

add_executable(static_file_test static_file_test.cpp)
target_link_libraries(static_file_test PRIVATE hp_http_core)
target_compile_options(static_file_test PRIVATE -Wall -Wextra -Wpedantic)
add_test(NAME static_file_test COMMAND static_file_test)
