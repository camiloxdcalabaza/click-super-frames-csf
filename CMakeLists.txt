cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(CFSPerfectFPS VERSION 1.0.1)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK no esta definido, compila con geode cli o con el action")
endif()

include($ENV{GEODE_SDK}/cmake/Geode.cmake)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

setup_geode_mod(${PROJECT_NAME})