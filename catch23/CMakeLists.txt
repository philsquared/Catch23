cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

add_library(Catch23 STATIC
        include/catch23/internal_test.h
        include/catch23/test.h
        include/catch23/test_result_handler.h
        src/test_result_handler.cpp
        src/internal_test.cpp
        src/test.cpp
        src/generators.cpp
        include/catch23/generators.h
        src/reporter.cpp
        include/catch23/reporter.h
        src/test_info.cpp
        include/catch23/test_info.h
        src/colour.cpp
        include/catch23/colour.h
        src/console_reporter.cpp
        include/catch23/console_reporter.h
        src/runner.cpp
        include/catch23/runner.h
        src/catch2.cpp
        include/catch23/catch2_compat.h
        src/internal_execution_nodes.cpp
        include/catch23/internal_execution_nodes.h
        src/sections.cpp
        include/catch23/sections.h
        include/catch23/random.h
        src/meta_test.cpp
        include/catch23/meta_test.h
        src/generator_node.cpp
        include/catch23/generator_node.h
        include/catch23/print.cpp
        include/catch23/print.h
        src/main.cpp
        include/catch23/main.h
        include/catch23/mocks.h
        src/mocks.cpp
)

target_include_directories(Catch23 PUBLIC include)
target_link_libraries(Catch23 PUBLIC Catchkit)
target_compile_options(Catch23 PRIVATE -Wall -Wextra -Wpedantic)
