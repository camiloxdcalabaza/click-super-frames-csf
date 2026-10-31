
cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Fixes para SDK 4.8.0 roto
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(GEODE_DISABLE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD OFF)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)

# FIX Android __synth3way
add_compile_definitions(
    _LIBCPP_HAS_NO_SPACESHIP=1
    __cpp_lib_three_way_comparison=0
    __cpp_impl_three_way_comparison=0
)

project(ClickSuperFramesOptimizer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Fuerza bindings sin unity y con fix de spaceship
if(TARGET GeodeBindings)
    set_target_properties(GeodeBindings PROPERTIES 
        UNITY_BUILD OFF
        UNITY_BUILD_BATCH_SIZE 0
    )
    target_compile_definitions(GeodeBindings PRIVATE
        _LIBCPP_HAS_NO_SPACESHIP=1
        __cpp_lib_three_way_comparison=0
    )
    # Fix iOS FunctionType - usa C++17 solo para bindings de iOS/macOS
    if(APPLE)
        set_target_properties(GeodeBindings PROPERTIES
            CXX_STANDARD 17
        )
    endif()
endif()

setup_geode_mod(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    _LIBCPP_HAS_NO_SPACESHIP=1
    __cpp_lib_three_way_comparison=0
)
