cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaulting to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(WITHSDL3_DEFAULT ON)
set(BUILD_WITH_SDL3 ${WITHSDL3_DEFAULT} CACHE BOOL "Build Application with SDL3 Demo")

set(WITH_FENSTER_DEFAULT ON)
set(BUILD_WITH_FENSTER ${WITH_FENSTER_DEFAULT} CACHE BOOL "Build Application with Fenster Demo")


set(SANITIZE_DEFAULT OFF)
set(BUILD_WITH_SANITIZE ${SANITIZE_DEFAULT} CACHE BOOL "Build Application with -fsanitize=address -g")

# -----------------------------------------------------------------------------
project(DreiZehn C CXX)

include ("${CMAKE_CURRENT_LIST_DIR}/src/DreiZehn.cmake")

set(PROJECT_SRC

    ext/linenoise/linenoise.c
    ext/linenoise/linenoise.h
    demo/main.cpp
)

# ------------- SDL3
set (SDL3_LINKLIB "")
if (BUILD_WITH_SDL3)
    find_package(SDL3 REQUIRED)
    set (SDL3_LINKLIB SDL3::SDL3)

    # list(APPEND PROJECT_SRC "engine/functions/SDL3Functions.h")
    add_compile_definitions(DREIZEHN_SDL3)
endif()

# ------------- FENSTER
set(FENSTER_LINKLIB "")
if (BUILD_WITH_FENSTER)
    if (WIN32)
        set(FENSTER_LINKLIB "gdi32;winmm")
    elseif (APPLE)
        set(FENSTER_LINKLIB "-framework Cocoa;-framework AudioToolbox")
    else()
        set(FENSTER_LINKLIB "X11;asound")
    endif()

    # list(APPEND PROJECT_SRC "engine/functions/FensterFunctions.h")
    # list(APPEND PROJECT_SRC "engine/functions/FensterAudioFunctions.h")
    add_compile_definitions(DREIZEHN_FENSTER)
endif()

# ------------- Main
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
    ${DREIZEHN_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)



target_include_directories(${PROJECT_NAME} PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/"
     "${CMAKE_CURRENT_SOURCE_DIR}/ext"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL3_LINKLIB}
    ${FENSTER_LINKLIB}
)

if (BUILD_WITH_SANITIZE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()
