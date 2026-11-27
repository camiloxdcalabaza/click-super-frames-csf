cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(CFSPerfectFPS VERSION 1.0.1)

# 1. Usa el SDK que ya instalo el Action
if(DEFINED ENV{GEODE_SDK} AND NOT "$ENV{GEODE_SDK}" STREQUAL "" AND EXISTS "$ENV{GEODE_SDK}/cmake/Geode.cmake")
    set(GEODE_SDK "$ENV{GEODE_SDK}")
# 2. Si te lo pasaron como -DGEODE_SDK=...
elseif(DEFINED GEODE_SDK AND EXISTS "${GEODE_SDK}/cmake/Geode.cmake")
    # ya esta en GEODE_SDK, no tocar
# 3. Si no hay nada, lo baja solo, sin compilarlo
else()
    include(FetchContent)
    FetchContent_Declare(
        GeodeSDK
        GIT_REPOSITORY https://github.com/geode-sdk/geode.git
        GIT_TAG v6.0.0
    )
    FetchContent_Populate(GeodeSDK)
    set(GEODE_SDK "${geodesdk_SOURCE_DIR}")
endif()

if(NOT DEFINED GEODE_SDK OR NOT EXISTS "${GEODE_SDK}/cmake/Geode.cmake")
    message(FATAL_ERROR "No se encontro Geode.cmake en: ${GEODE_SDK}")
endif()

include(${GEODE_SDK}/cmake/Geode.cmake)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

setup_geode_mod(${PROJECT_NAME})