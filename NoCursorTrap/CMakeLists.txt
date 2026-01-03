cmake_minimum_required(VERSION 4.0)
project(NoCursorTrap VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(NoCursorTrap SHARED
        src/plugin.cpp
)

target_include_directories(NoCursorTrap PRIVATE
        src
        includes
)

target_link_libraries(NoCursorTrap PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bins/JAPI.dll)