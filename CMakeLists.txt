cmake_minimum_required(VERSION 3.14)

project(Maglev
        VERSION 1.0
        DESCRIPTION "Maglev consistent hasher and dynamic load balancer.")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
SET(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKECONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}")

option(BUILD_TEST "Build test." FALSE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if (BUILD_TEST)
    enable_testing()
    add_subdirectory(test/unit_test)
    add_subdirectory(test/performance_test)
endif()

configure_file("${PROJECT_NAME}Config.cmake.in" "${PROJECT_NAME}Config.cmake"
        @ONLY)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/maglev"
        DESTINATION ${INCLUDE_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
