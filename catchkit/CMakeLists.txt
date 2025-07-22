cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 23)

add_library(Catchkit STATIC
        src/catchkit_checker.cpp
        src/catchkit_result_handler.cpp
        src/catchkit_result_type.cpp
        src/catchkit_operators.cpp
        src/catchkit_expression_info.cpp
        src/catchkit_assertion_context.cpp
        include/catchkit/catchkit_checker.h
        include/catchkit/catchkit_internal_warnings.h
        include/catchkit/catchkit_unique_name.h
        include/catchkit/catchkit_result_handler.h
        include/catchkit/catchkit_result_type.h
        include/catchkit/catchkit_operators.h
        include/catchkit/catchkit_expression_info.h
        include/catchkit/catchkit_assertion_context.h
        src/catchkit_assert_result_handler.cpp
        include/catchkit/catchkit_assert_result_handler.h
        src/catchkit_expr_ref.cpp
        include/catchkit/catchkit_expr_ref.h
        src/catchkit_operator_to_string.cpp
        include/catchkit/catchkit_operator_to_string.h
        src/catchkit_decomposer.cpp
        include/catchkit/catchkit_decomposer.h
        src/catchkit_exceptions.cpp
        include/catchkit/catchkit_exceptions.h
        src/catchkit_stringify.cpp
        include/catchkit/catchkit_stringify.h
        src/catchkit_matchers.cpp
        include/catchkit/catchkit_matchers.h
        include/catchkit/catchkit_platform.h)

target_include_directories(Catchkit PUBLIC include)
target_compile_options(Catchkit PRIVATE -Wall -Wextra -Wpedantic)
