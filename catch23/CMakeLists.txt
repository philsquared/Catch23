cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

add_library(Catch23 STATIC
        include/catch23/catch23_internal_test.h
        include/catch23/catch23_test.h
        include/catch23/catch23_test_result_handler.h
        src/catch23_test_result_handler.cpp
        src/catch23_internal_test.cpp
        src/catch23_test.cpp
        src/catch23_generators.cpp
        include/catch23/catch23_generators.h
        src/catch23_reporter.cpp
        include/catch23/catch23_reporter.h
        src/catch23_test_info.cpp
        include/catch23/catch23_test_info.h
        src/catch23_colour.cpp
        include/catch23/catch23_colour.h
        src/catch23_console_reporter.cpp
        include/catch23/catch23_console_reporter.h
        src/catch23_runner.cpp
        include/catch23/catch23_runner.h
        src/catch23_catch2.cpp
        include/catch23/catch23_catch2.h
        src/catch23_internal_execution_nodes.cpp
        include/catch23/catch23_internal_execution_nodes.h
        src/catch23_sections.cpp
        include/catch23/catch23_sections.h
        include/catch23/catch23_random.h
        include/catch23/catch23_local_test.cpp
        include/catch23/catch23_local_test.h
        src/catch23_generator_node.cpp
        include/catch23/catch23_generator_node.h
)

target_include_directories(Catch23 PUBLIC include)
target_link_libraries(Catch23 PUBLIC Catchkit)
target_compile_options(Catch23 PRIVATE -Wall -Wextra -Wpedantic)
