cmake_minimum_required(VERSION 3.14)

project(installation_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Maglev REQUIRED)

add_executable(installation_test "main.cpp")

target_include_directories(installation_test PUBLIC ${Maglev_INCLUDE_DIR})

enable_testing()

add_test(NAME installation_test COMMAND installation_test)

