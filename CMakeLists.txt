cmake_minimum_required(VERSION 3.21)
project(CFSPerfectFPS VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK no esta definido. La Action de Geode lo tiene que setear.")
endif()

include($ENV{GEODE_SDK}/cmake/Geode.cmake)

setup_geode_mod(${PROJECT_NAME})