cmake_minimum_required(VERSION 3.14)

project(Maglev
        VERSION 1.0
        DESCRIPTION "Maglev consistent hasher and load balancer.")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB_RECURSE SOURCE_FILES "include/*.h" "include/*/*.h")

option(BUILD_TEST "Build test." FALSE)
if (BUILD_TEST)
    enable_testing()
    add_subdirectory(test/unit_test)
    add_subdirectory(test/performance_test)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
