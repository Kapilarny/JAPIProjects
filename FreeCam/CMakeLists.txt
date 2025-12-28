cmake_minimum_required(VERSION 4.0)
project(FreeCam VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(FreeCam SHARED
        src/plugin.cpp
)

target_include_directories(FreeCam PRIVATE
        src
        includes
)

target_link_libraries(FreeCam PRIVATE
        ${CMAKE_SOURCE_DIR}/bins/JAPI.dll)
