cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
# Project settings - this is all you have to modify:
# project(baseElf LANGUAGES C CXX)
project(BaseFlux13)
set(PROJECT_SRC
    src/main.cpp
    src/bindings.cpp
    # src/bindings/FileDialog.cpp
)

set(PROJECT_INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/src/
)

set (ASSET_PATH ${CMAKE_CURRENT_LIST_DIR}/assets@assets)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# -----------------------------------------------------------------------------
# -----------------------------------------------------------------------------
set(SANITIZE_DEFAULT OFF)
set(BUILD_WITH_SANITIZE ${SANITIZE_DEFAULT} CACHE BOOL "Build Application with -fsanitize=address -g")
# -----------------------------------------------------------------------------

if (EMSCRIPTEN)
    add_link_options(
        "-sALLOW_MEMORY_GROWTH=1"
    )
endif()

include(FetchContent)
# ------------- BaseFlux
#  --- BaseFlux from Directory OR GITHUB ---
set(BASEFLUX_DIR "/opt/BaseFlux" CACHE PATH "Path to BaseFlux")
if(NOT EXISTS "${BASEFLUX_DIR}/CMakeLists.txt")
  #  message(FATAL_ERROR "The directory OHMFLUX_DIR='${OHMFLUX_DIR}' does not contain a CMakeLists.txt file!")
  FetchContent_Declare(
    BaseFlux
    # --- directly from github:
    GIT_REPOSITORY https://github.com/ohmtal/BaseFlux.git
    GIT_TAG        main
  )
  FetchContent_MakeAvailable(BaseFlux)

else()
  add_subdirectory("${BASEFLUX_DIR}" "${CMAKE_BINARY_DIR}/baseFlux_build")
endif()

set (PROJECT_LIBS BaseFlux)

# =============== DreiZehn >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
set(DREIZEHN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../" CACHE PATH "Path to DreiZehn Script")
set(DREIZEHN_CMAKE ${DREIZEHN_PATH}/src/DreiZehn.cmake)


if(NOT EXISTS ${DREIZEHN_CMAKE})
    message(FATAL_ERROR
        "\n DreiZehnScript path is not set or invalid: ${DREIZEHN_PATH} "
        "\n Please provide the path using: "
        "\n cmake -B build -DDREIZEHN_PATH=/path/to/DreiZehnScript"
    )
else()
 include(${DREIZEHN_CMAKE})
 list(APPEND PROJECT_INCLUDES "${DREIZEHN_PATH}/src/")
endif()


# <<<<<<<<<<<<<<<<<<<<<<<<<<<< DreiZehnScript ========================


# ------------- Main
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
    ${DREIZEHN_SRC}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  IMGUI_DEFINE_MATH_OPERATORS
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"

    # -- for windows: 
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_INCLUDES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_LIBS}
)


# Windows - winVolume :: need to set on all builds 
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/Zc:wchar_t-")
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        NOMINMAX 
        UNICODE 
        _UNICODE
    )
endif()

# Web Configurations
if (EMSCRIPTEN)

    add_link_options(
        "-sALLOW_MEMORY_GROWTH=1"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PUBLIC --sUSE_SDL=3 PUBLIC --preload-file ${ASSET_PATH} PRIVATE "-lidbfs.js")

    set(TARGET_NAME ${PROJECT_NAME})

    set(CUSTOM_SHELL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/res/minshell.html")
    if(EXISTS "${CUSTOM_SHELL_FILE}")
        target_link_options(${TARGET_NAME} PRIVATE "--shell-file" "${CUSTOM_SHELL_FILE}")
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS "${CUSTOM_SHELL_FILE}")
    endif()

endif()


if (BUILD_WITH_SANITIZE)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -g)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif()
