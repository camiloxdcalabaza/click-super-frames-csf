cmake_minimum_required(VERSION 3.21)
project(CFSPerfectFPS VERSION 1.0.0)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK no esta definido")
endif()

# ESTA ES LA LINEA QUE TE FALTA, VA ANTES DE setup_geode_mod
include($ENV{GEODE_SDK}/cmake/Geode.cmake)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if(NOT DEFINED CMAKE_CXX_VISIBILITY_PRESET)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

setup_geode_mod(${PROJECT_NAME})