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


set(LOADALL_DEFAULT ON)
set(BUILD_WITH_LOADALL ${LOADALL_DEFAULT} CACHE BOOL "Build Application with load all available modules at start")

# -----------------------------------------------------------------------------
include ("${CMAKE_CURRENT_LIST_DIR}/src/DreiZehn.cmake")

if (BUILD_WITH_LOADALL)
    add_compile_definitions(DREIZEHN_LOADALL_MODULES)
endif()

add_library(DreiZehn
    ${DREIZEHN_SRC}
)

target_include_directories(DreiZehn PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/"
     "${CMAKE_CURRENT_SOURCE_DIR}/src/"
)


if(WIN32 AND MSVC)
    add_compile_options(/MP)
endif()
