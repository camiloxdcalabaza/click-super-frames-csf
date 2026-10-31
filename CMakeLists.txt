cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# FIX bindings rotos
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(CMAKE_UNITY_BUILD OFF)
set_property(GLOBAL PROPERTY UNITY_BUILD OFF)

project(ClickSuperFramesOptimizer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Desactiva Unity tambien para los bindings
if(TARGET GeodeBindings)
    set_target_properties(GeodeBindings PROPERTIES UNITY_BUILD OFF)
endif()

setup_geode_mod(${PROJECT_NAME})
