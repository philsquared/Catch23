cmake_minimum_required(VERSION 3.28)
project(Catch23Test)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(catch23)
add_subdirectory(catchkit)

add_executable(Catch23Test src/main.cpp src/UsageTests/Condition.tests.cpp
        src/UsageTests/Matchers.tests.cpp
        src/IntrospectionTests/ExecutionNodes.tests.cpp
        src/UsageTests/Section.tests.cpp
        src/UsageTests/ToStringVector.tests.cpp
        src/UsageTests/Generator.tests.cpp
        src/IntrospectionTests/Meta.tests.cpp
        src/UsageTests/PropertyBased.tests.cpp
        src/UsageTests/ToStringPair.tests.cpp
        src/IntrospectionTests/Reflection.tests.cpp
        src/UsageTests/Stringify.tests.cpp
        src/UsageTests/Tricky.tests.cpp)

target_link_libraries(Catch23Test PUBLIC Catch23)
target_link_libraries(Catch23Test PUBLIC Catchkit)
target_compile_options(Catch23Test PRIVATE -Wall -Wextra -Wpedantic)

# If modules are enabled, define USE_CATCH23_MODULES for test files
if(CATCH23_BUILD_MODULES)
    target_compile_definitions(Catch23Test PRIVATE USE_CATCH23_MODULES)
endif()
