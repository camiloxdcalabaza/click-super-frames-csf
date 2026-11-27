cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

if (IOS)
    set(CMAKE_OSX_ARCHITECTURES arm64)
elseif (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(click-super-frames VERSION 1.0.1)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK no definido, la action de Geode lo tiene que poner sola")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})