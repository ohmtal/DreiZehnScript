    cmake_minimum_required(VERSION 3.24)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FORCE_02 ON)
set(BUILD_WITH_FORCEO2 ${FORCE_02} CACHE BOOL "Build Application Release also with -02 on gcc")
# -02 is faster on gcc (StarField2 > 300fps instead of 250fps)!!!
if(NOT MSVC AND BUILD_WITH_FORCEO2)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

set(DREIZEHN_SRC

    # core
    ${CMAKE_CURRENT_LIST_DIR}/core/AstNode.h
    ${CMAKE_CURRENT_LIST_DIR}/core/AstNode.cpp

    ${CMAKE_CURRENT_LIST_DIR}/core/FunctionMap.h
    ${CMAKE_CURRENT_LIST_DIR}/core/FunctionMap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/GarbageCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/core/GarbageCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Lexer.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Lexer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Parser.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Value.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Value.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/functions/VectorFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/PointVectorObjectFunctions.h

    ${CMAKE_CURRENT_LIST_DIR}/functions/SysFunctions.h
    ${CMAKE_CURRENT_LIST_DIR}/functions/SysFunctions.cpp


    # ToolBox
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/SymbolTable.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/Tools.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/Tools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/fluxStr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/fluxStr.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/SysTools.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/SysTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/PixelBuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/PixelBuffer.cpp

    # Bytecode compiler
    ${CMAKE_CURRENT_LIST_DIR}/bytecode/CompilerScope.h
    ${CMAKE_CURRENT_LIST_DIR}/bytecode/VM.h
    ${CMAKE_CURRENT_LIST_DIR}/bytecode/VM.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bytecode/VMStructure.h
    ${CMAKE_CURRENT_LIST_DIR}/bytecode/VMStructure.cpp


    ${CMAKE_CURRENT_LIST_DIR}/Globals.h
    ${CMAKE_CURRENT_LIST_DIR}/DreiZehn.h

)
