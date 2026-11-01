cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(WITHSDL3_DEFAULT ON)
set(BUILD_WITH_SDL3 ${WITHSDL3_DEFAULT} CACHE BOOL "Build Application with SDL3 Demo")


# -----------------------------------------------------------------------------
project(dlsSDL C CXX)


set (LINK_LIBS)

set(PROJECT_SRC
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SDL3Functions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/functions/MathFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/functions/StringFunctions.cpp
)

# ------------- SDL3
if (BUILD_WITH_SDL3)
    find_package(SDL3 REQUIRED)

    list(APPEND LINK_LIBS SDL3::SDL3)
    list(APPEND PROJECT_SRC "SDL3Functions.cpp")
    add_compile_definitions(DREIZEHN_SDL3)
endif()

# ------------- Main
add_executable(${PROJECT_NAME}
    ${PROJECT_SRC}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
     "${CMAKE_CURRENT_SOURCE_DIR}/"
     "${CMAKE_CURRENT_SOURCE_DIR}/../src"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LINK_LIBS}
)

