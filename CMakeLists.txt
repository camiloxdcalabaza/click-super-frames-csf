cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(CFSPerfectFPS VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if(NOT DEFINED CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "No compiler defined")
endif()

# ESTA ES LA LINEA QUE TE ESTABA ROMPIENDO TODO
# Antes tenias: D:/.../geode-sdk-clone/cmake/Geode.cmake
# Ahora usa el SDK que instala GitHub Actions
if(DEFINED ENV{GEODE_SDK})
    include($ENV{GEODE_SDK}/cmake/Geode.cmake)
else()
    # Fallback por si lo compilas a mano en tu PC
    include(cmake/Geode.cmake)
endif()

setup_geode_mod(${PROJECT_NAME})