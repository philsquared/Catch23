cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)

add_library(Catchkit STATIC
        src/checker.cpp
        src/result_handler.cpp
        src/result_type.cpp
        src/operators.cpp
        src/expression_info.cpp
        src/assertion_context.cpp
        include/catchkit/checker.h
        include/catchkit/internal_warnings.h
        include/catchkit/unique_name.h
        include/catchkit/result_handler.h
        include/catchkit/result_type.h
        include/catchkit/operators.h
        include/catchkit/expression_info.h
        include/catchkit/assertion_context.h
        src/assert_result_handler.cpp
        include/catchkit/assert_result_handler.h
        src/expr_ref.cpp
        include/catchkit/expr_ref.h
        src/operator_to_string.cpp
        include/catchkit/operator_to_string.h
        src/exceptions.cpp
        include/catchkit/exceptions.h
        src/stringify.cpp
        include/catchkit/stringify.h
        src/matchers.cpp
        include/catchkit/matchers.h
        include/catchkit/platform.h
        include/catchkit/report_on.h
        src/internal_matchers.cpp
        include/catchkit/internal_matchers.h
        src/variable_capture.cpp
        include/catchkit/variable_capture.h
)

target_include_directories(Catchkit PUBLIC include)
target_compile_options(Catchkit PRIVATE -Wall -Wextra -Wpedantic)
