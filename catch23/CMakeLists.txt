cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

add_library(Catch23 STATIC
        include/catch23/catch23_internal_test.h
        include/catch23/catch23_test.h
        include/catch23/catch23_test_result_handler.h
        src/catch23_test_result_handler.cpp
        src/catch23_internal_test.cpp
        src/catch23_test.cpp
        include/catch23/catch23_generators.cpp
        include/catch23/catch23_generators.h)

target_include_directories(Catch23 PUBLIC include)
target_link_libraries(Catch23 PUBLIC Catchkit)
target_compile_options(Catch23 PRIVATE -Wall -Wextra -Wpedantic)
