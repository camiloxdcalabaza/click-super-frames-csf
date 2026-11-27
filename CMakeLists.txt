cmake_minimum_required(VERSION 3.21)
project(ClickSuperFrames VERSION 1.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# La action de Geode ya instala el SDK y deja GEODE_SDK seteado.
# NO usar FetchContent / CPM aca, porque pisa / duplica el SDK.
if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "No se encontro GEODE_SDK. Compila con 'geode build' o con la action build-geode-mod.")
endif()

# En Geode 6 el archivo esta en cmake/Geode.cmake dentro del SDK
include($ENV{GEODE_SDK}/cmake/Geode.cmake)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

# Esto es lo que crea el .geode, linkea bindings, etc.
setup_geode_mod(${PROJECT_NAME})