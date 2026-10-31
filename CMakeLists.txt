cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_UNITY_BUILD OFF)
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(GEODE_DISABLE_UNITY_BUILD ON)

# iOS + macOS necesitan C++17 por FunctionType
if(APPLE)
    set(CMAKE_CXX_STANDARD 17)
endif()

project(ClickSuperFramesOptimizer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

# Android32 + Android64 necesitan el shim __synth3way_t
if(ANDROID)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/android_fix.hpp
"#pragma once
namespace std{namespace __detail{template<class... T> using __synth3way_t=int; template<class A,class B> inline int __synth3way(A&&,B&&){return 0;}}}"
    )
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_options(${PROJECT_NAME} PRIVATE -include ${CMAKE_CURRENT_BINARY_DIR}/android_fix.hpp)
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Windows necesita esto por LoadingCircle / AchievementBar
if(TARGET GeodeBindings)
    set_target_properties(GeodeBindings PROPERTIES UNITY_BUILD OFF)
endif()

setup_geode_mod(${PROJECT_NAME})
