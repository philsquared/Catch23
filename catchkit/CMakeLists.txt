cmake_minimum_required(VERSION 3.28)

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
        include/catchkit/internal_macro_utils.h
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
        include/catchkit/internal_platform.h
        include/catchkit/report_on.h
        src/internal_matchers.cpp
        include/catchkit/internal_matchers.h
        src/variable_capture.cpp
        include/catchkit/variable_capture.h
        src/reflection.cpp
        include/catchkit/reflection.h
        include/catchkit/internal_compilers.h
        src/internal_compilers.cpp
        src/internal_platform.cpp
)

target_include_directories(Catchkit PUBLIC include)
target_compile_options(Catchkit PRIVATE -Wall -Wextra -Wpedantic)

# Optional C++20 module support
# Enable with: cmake -B build -DCATCHKIT_BUILD_MODULES=ON
option(CATCHKIT_BUILD_MODULES "Build CatchKit as a C++20 module" OFF)

if(CATCHKIT_BUILD_MODULES)
    target_sources(Catchkit PUBLIC
        FILE_SET CXX_MODULES FILES
        modules/catchkit.cppm
    )
endif()
