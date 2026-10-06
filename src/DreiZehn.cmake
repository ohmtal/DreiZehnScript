    cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(DREIZEHN_SRC

    # core
    ${CMAKE_CURRENT_LIST_DIR}/core/AstNode.h
    ${CMAKE_CURRENT_LIST_DIR}/core/AstNode.cpp

    ${CMAKE_CURRENT_LIST_DIR}/core/FunctionMap.h
    ${CMAKE_CURRENT_LIST_DIR}/core/FunctionMap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Lexer.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Parser.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Value.h
    ${CMAKE_CURRENT_LIST_DIR}/core/ValueObject.h
    ${CMAKE_CURRENT_LIST_DIR}/core/ValueObject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/VariableFrame.h

    ${CMAKE_CURRENT_LIST_DIR}/Environment.h

    ${CMAKE_CURRENT_LIST_DIR}/ScriptLoader.h
    ${CMAKE_CURRENT_LIST_DIR}/ScriptLoader.cpp


    # Functions and objects
    ${CMAKE_CURRENT_LIST_DIR}/functions/CoreFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/StringFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/MathFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/DebugFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/ArrayFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/VectorFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/PointVectorObjectFunctions.h


    # ToolBox
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/SymbolTable.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/Tools.h

    # Bytecode compiler TODO
    # ${CMAKE_CURRENT_LIST_DIR}/bytecode/AstNode_bytecode.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/bytecode/ASTCompiler.h
    # ${CMAKE_CURRENT_LIST_DIR}/bytecode/CompilerScope.h
    # ${CMAKE_CURRENT_LIST_DIR}/bytecode/VM.h
    # ${CMAKE_CURRENT_LIST_DIR}/bytecode/VMStructure.h


    ${CMAKE_CURRENT_LIST_DIR}/Globals.h
    ${CMAKE_CURRENT_LIST_DIR}/DreiZehn.h

)

# # ------------- SDL3
# set (SDL3_LINKLIB "")
# if (BUILD_WITH_SDL3)
#     find_package(SDL3 REQUIRED)
#     set (SDL3_LINKLIB SDL3::SDL3)
#
#     list(APPEND PROJECT_SRC "${CMAKE_CURRENT_LIST_DIR}/functions/SDL3Functions.h")
#     add_compile_definitions(DREIZEHN_SDL3)
# endif()
#
# # ------------- FENSTER
# set(FENSTER_LINKLIB "")
# if (BUILD_WITH_FENSTER)
#     if (WIN32)
#         set(FENSTER_LINKLIB "gdi32;winmm")
#     elseif (APPLE)
#         set(FENSTER_LINKLIB "-framework Cocoa;-framework AudioToolbox")
#     else()
#         set(FENSTER_LINKLIB "X11;asound")
#     endif()
#
#     list(APPEND PROJECT_SRC "${CMAKE_CURRENT_LIST_DIR}/functions/FensterFunctions.h")
#     list(APPEND PROJECT_SRC "${CMAKE_CURRENT_LIST_DIR}/functions/FensterAudioFunctions.h")
#     add_compile_definitions(DREIZEHN_FENSTER)
# endif()


