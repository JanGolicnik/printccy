cmake_minimum_required(VERSION 3.16)
project(printccy)

add_library(printccy INTERFACE)

target_include_directories(printccy
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)