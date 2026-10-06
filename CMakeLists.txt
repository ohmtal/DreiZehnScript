cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
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

set(WITH_RAYLIB_DEFAULT ON)
set(BUILD_WITH_RAYLIB ${WITH_RAYLIB_DEFAULT} CACHE BOOL "Build Application with Raylib Demo")

set(WITH_MA_DEFAULT OFF)
set(BUILD_WITH_MA ${WITH_MA_DEFAULT} CACHE BOOL "Build Application with MiniAudio")


set(SANITIZE_DEFAULT OFF)
set(BUILD_WITH_SANITIZE ${SANITIZE_DEFAULT} CACHE BOOL "Build Application with -fsanitize=address -g")

set(LOADALL_DEFAULT ON)
set(BUILD_WITH_LOADALL ${LOADALL_DEFAULT} CACHE BOOL "Build Application with load all available modules at start")
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

# ------------- RayLib
set(RAYLIB_LINKLIB "")
if (BUILD_WITH_RAYLIB)
    set(RAYLIB_DIR "${CMAKE_CURRENT_LIST_DIR}/ext/raylib-6.0/" CACHE PATH "Path to RayLib")
    message( STATUS "\n Raylib directory is: " ${RAYLIB_DIR} )
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
    set(BUILD_GAMES    OFF CACHE BOOL "Build raylib games"    FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    add_subdirectory("${RAYLIB_DIR}" "${CMAKE_BINARY_DIR}/raylib_build")

    set(RAYLIB_LINKLIB "raylib")

    add_compile_definitions(DREIZEHN_RAYLIB)
endif()

# ------------- MiniAudio
set(MA_LINKLIB "")
if (BUILD_WITH_MINIAUDIO)
    set(MA_SRC
        ${CMAKE_CURRENT_LIST_DIR}/ext/miniaudio/miniaudio.c
        ${CMAKE_CURRENT_LIST_DIR}/ext/miniaudio/miniaudio.h
    )
    list(APPEND PROJECT_SRC ${MA_SRC})

    if (WIN32)
        # set(MA_LINKLIB "winmm")
    elseif (APPLE)
        set(MA_LINKLIB "-framework CoreAudio" "-framework CoreFoundation" "-framework AudioToolbox")
    else()
        set(MA_LINKLIB pthread m dl)
    endif()

    add_compile_definitions(DREIZEHN_MINIAUDIO)

endif()

if (BUILD_WITH_LOADALL)
    add_compile_definitions(DREIZEHN_LOADALL_MODULES)
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
    ${RAYLIB_LINKLIB}
    ${MA_LINKLIB}
)

if (BUILD_WITH_SANITIZE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()
