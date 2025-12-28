cmake_minimum_required(VERSION 4.0)
project(SkipIntro VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(SkipIntro SHARED
        src/plugin.cpp
)

target_include_directories(SkipIntro PRIVATE
        src
        includes
)

target_link_libraries(SkipIntro PRIVATE
        ${CMAKE_SOURCE_DIR}/bins/JAPI.dll)
