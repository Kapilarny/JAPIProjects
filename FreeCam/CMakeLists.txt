cmake_minimum_required(VERSION 4.0)
project(FreeCam VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_SHARED_LIBRARY_PREFIX "")

option(STATIC_CPP_RUNTIME "Link C/C++ runtime statically" OFF)

if (MSVC AND STATIC_CPP_RUNTIME)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif((CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang") AND STATIC_CPP_RUNTIME)
    add_link_options(
            -static-libstdc++
            -static-libgcc
    )
endif()

# Disable tail call optimization to improve stack traces + make API calls not bug out in some cases
if (MSVC)
    add_compile_options(/Oy-)
else()
    add_compile_options(-fno-optimize-sibling-calls)
endif()


add_library(FreeCam SHARED
        src/plugin.cpp
)

target_include_directories(FreeCam PRIVATE
        src
        includes
)

target_link_libraries(FreeCam PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bins/JAPI.dll
        ${CMAKE_CURRENT_SOURCE_DIR}/bins/imgui_shared.dll
)
