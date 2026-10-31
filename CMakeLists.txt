cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Desactiva unity y PCH que rompe AchievementBar, LoadingCircle, etc
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)
set(GEODE_DISABLE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD OFF)
set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)

# iOS/macOS: FunctionType fix - usa C++17 solo en Apple
if(APPLE)
    set(CMAKE_CXX_STANDARD 17)
endif()

project(ClickSuperFramesOptimizer VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
endif()

# --- PARCHE AUTOMATICO __synth3way para Android NDK r26 ---
if(EXISTS "$ENV{GEODE_SDK}/loader/include/Geode/c++stl/gnustl")
    file(GLOB_RECURSE _gnustl_headers "$ENV{GEODE_SDK}/loader/include/Geode/c++stl/gnustl/*.h")
    foreach(_h IN LISTS _gnustl_headers)
        file(READ ${_h} _content)
        if(_content MATCHES "__synth3way")
            string(REPLACE "std::__detail::__synth3way(__lhs.base(), __rhs.base())" "(__lhs.base() == __rhs.base() ? 0 : (__lhs.base() < __rhs.base() ? -1 : 1))" _content "${_content}")
            string(REGEX REPLACE "std::__detail::__synth3way\\([^)]+\\)" "0" _content "${_content}")
            file(WRITE ${_h} "${_content}")
        endif()
    endforeach()
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Fuerza bindings sin unity
if(TARGET GeodeBindings)
    set_target_properties(GeodeBindings PROPERTIES 
        UNITY_BUILD OFF
        UNITY_BUILD_BATCH_SIZE 0
    )
endif()

setup_geode_mod(${PROJECT_NAME})
