cmake_minimum_required(VERSION 3.21)
project(CFSPerfectFPS VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Esta es la parte que estaba mal:
if(DEFINED ENV{GEODE_SDK})
    include($ENV{GEODE_SDK}/cmake/Geode.cmake)
else()
    # Fallback por si lo compilás a mano con el SDK clonado al lado
    include(${CMAKE_CURRENT_SOURCE_DIR}/../geode-sdk/cmake/Geode.cmake)
endif()

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK no está seteado")
endif()

setup_geode_mod(${PROJECT_NAME})