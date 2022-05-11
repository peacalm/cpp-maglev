cmake_minimum_required(VERSION 3.14)

project(maglev)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
if (NOT GTEST_FOUND)
    message(FATAL_ERROR "GTest not found! Please install GTest first.")
endif()

option(ENABLE_MYOSTREAM_WATCH "Use lib myostream to print variables to console." FALSE)
if (ENABLE_MYOSTREAM_WATCH)
    add_definitions(-DENABLE_MYOSTREAM_WATCH)
endif()

file(GLOB SOURCE_FILES "*.cpp")
add_executable(unit_test ${SOURCE_FILES})

target_include_directories(unit_test PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_include_directories(unit_test PUBLIC ${GTEST_INCLUDE_DIR})
target_link_libraries(unit_test PUBLIC ${GTEST_LIBRARIES})

add_test(NAME unit_test COMMAND unit_test)
