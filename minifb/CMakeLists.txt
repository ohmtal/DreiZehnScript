cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaulting to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DLS_DEBUG)
endif()

# -----------------------------------------------------------------------------
project(pixels C CXX)

set(DREIZEHN_LINENOISE ON CACHE BOOL "" FORCE)
include ("../src/DreiZehn.cmake")
set (LINK_LIBS DreiZehn)

set(PROJECT_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/toolbox/PixelBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../addons/minifb/MinifbFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../addons/Miniaudio/miniaudio/miniaudio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../addons/Miniaudio/MiniAudioFunctions.cpp
)

# ------------- miniFB

include(FetchContent)

set(MINIFB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MINIFB_BUILD_VERSION_INFO OFF CACHE BOOL "" FORCE)
# set(MINIFB_USE_WAYLAND_API ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    minifb
    GIT_REPOSITORY https://github.com/emoon/minifb.git
    GIT_TAG        v0.16.0
)

FetchContent_MakeAvailable(minifb)

list(APPEND LINK_LIBS minifb)

# ------------- Main
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../

)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LINK_LIBS}
)

