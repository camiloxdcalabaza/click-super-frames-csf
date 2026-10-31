cmake_minimum_required(VERSION 3.21)

# TIENE que ir antes de project() y en CACHE
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_UNITY_BUILD OFF)
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON CACHE BOOL "" FORCE)
set(GEODE_DISABLE_UNITY_BUILD ON CACHE BOOL "" FORCE)

if(APPLE)
    set(CMAKE_CXX_STANDARD 17)
endif()

project(ClickSuperFramesOptimizer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

# FIX ANDROID32/64 - GLOBAL
if(ANDROID)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/android_fix.hpp
"#pragma once
namespace std{namespace __detail{template<class... T> using __synth3way_t=int; template<class A,class B> inline int __synth3way(A&&,B&&){return 0;}}}"
    )
    add_compile_options(-include ${CMAKE_CURRENT_BINARY_DIR}/android_fix.hpp)
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# FIX WINDOWS - LoadingCircle / GJGameLevel incomplete type
if(TARGET GeodeBindings)
    set_target_properties(GeodeBindings PROPERTIES UNITY_BUILD OFF UNITY_BUILD_BATCH_SIZE 0)
endif()

setup_geode_mod(${PROJECT_NAME})
