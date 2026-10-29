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

# set(WITH_LINENOISE_DEFAULT ON)
# set(BUILD_WITH_LINENOISE ${WITH_LINENOISE_DEFAULT} CACHE BOOL "Enable interactice console support.")


set(SANITIZE_DEFAULT OFF)
set(BUILD_WITH_SANITIZE ${SANITIZE_DEFAULT} CACHE BOOL "Build Application with -fsanitize=address -g")


# -----------------------------------------------------------------------------
project(dlsDemo C CXX)

set (LINK_LIBS)

set(PROJECT_SRC

    demo/main.cpp
)


# if (BUILD_WITH_LINENOISE)
#     add_compile_definitions(DREIZEHN_LINENOISE)
# endif()

# ------------- Main
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)



target_include_directories(${PROJECT_NAME} PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/src/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LINK_LIBS}
)

if (BUILD_WITH_SANITIZE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()

if(WIN32 AND MSVC)

    # my SDL3 line:
    # ===========
    # cmake -B build -S . `
    # -DCMAKE_TOOLCHAIN_FILE="C:/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" `
    # -DVCPKG_TARGET_TRIPLET=x64-windows `
    # -A x64 -DBUILD_WITH_FENSTER=OFF -DBUILD_WITH_RAYLIB=OFF

   set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_options(${PROJECT_NAME}  PRIVATE "/Zc:wchar_t-")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        NOMINMAX
        UNICODE
        _UNICODE
    )

    if (BUILD_WITH_RAYLIB)
        target_compile_definitions(${PROJECT_NAME} PRIVATE NOGDI NOUSER)
    endif()

    add_compile_options(/MP)
endif()

