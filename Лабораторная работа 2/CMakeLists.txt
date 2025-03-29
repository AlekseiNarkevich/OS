cmake_minimum_required(VERSION 3.12)
project(Lab2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab2_CPP14
    src/main.cpp
    src/average.cpp
    src/min_max.cpp
    src/data.cpp
)

target_include_directories(Lab2_CPP14 PRIVATE include)
set_target_properties(Lab2_CPP14 PROPERTIES CXX_STANDARD 14)

add_executable(Lab2_CPP98
    src/main.cpp
    src/average.cpp
    src/min_max.cpp
    src/data.cpp
)

target_include_directories(Lab2_CPP98 PRIVATE include)
set_target_properties(Lab2_CPP98 PROPERTIES CXX_STANDARD 98)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

add_executable(Lab2_Tests14
    test/test14.cpp
    src/average.cpp
    src/min_max.cpp
    src/data.cpp
)

target_include_directories(Lab2_Tests14 PRIVATE include)
set_target_properties(Lab2_Tests14 PROPERTIES CXX_STANDARD 14)
target_link_libraries(Lab2_Tests14 PRIVATE gtest_main)

add_executable(Lab2_Tests98
    test/test98.cpp
    src/average.cpp
    src/min_max.cpp
    src/data.cpp
)

target_include_directories(Lab2_Tests98 PRIVATE include)
set_target_properties(Lab2_Tests98 PROPERTIES CXX_STANDARD 98)
target_link_libraries(Lab2_Tests98 PRIVATE gtest_main)