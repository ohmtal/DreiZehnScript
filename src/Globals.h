//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#pragma once

#include "toolbox/Tools.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <cstdint>

#if defined(_MSC_VER)
// Microsoft Visual Studio Compiler
#define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
// GCC, Clang, and Intel Compiler (on Linux/macOS)
#define FORCE_INLINE inline __attribute__((always_inline))
#else
// Fallback for other compilers
#define FORCE_INLINE inline
#endif

namespace DreiZehn {
    class Environment;
    class VariableFrame;

    // dont want to break my fingers every time
    typedef uint32_t U32;
    typedef int64_t  S64;
    typedef int32_t  S32;
    typedef float    F32;
    typedef double   F64;

    const double EPSILON = 1e-9;


    inline std::function<bool()> OnBreath = nullptr;
    // bool BaseBreath();



    inline bool BaseBreath() {
        if (OnBreath) {
            return OnBreath();
        }
        return true;
    }



namespace Globals {
    inline bool ByteCodeMode = false;
    inline bool ByteCodeRunning = false;

    inline int currentScriptLineNumber = 0;
    inline std::string currentScriptLine =  "";
    // ------
    inline bool shutDownRUNNING = false;
    // ------
    // i want to track the current Env!
    inline Environment* gCurEnv = nullptr;
    inline Environment* gMainEnv = nullptr;

    inline VariableFrame* MasterVariableFrame = nullptr;

    // as sorted list for fast binary_search
    inline std::vector<uint32_t> GlobalVarList;
    inline bool isGlobalVar(uint32_t id) {
          return std::binary_search(GlobalVarList.begin(), GlobalVarList.end(), id);
    }
    inline bool insertGlobalVar(uint32_t id) {
        if (isGlobalVar(id)) return false;
        GlobalVarList.push_back(id);

        // should be done once but i want to be sure it's done!
        std::sort(GlobalVarList.begin(), GlobalVarList.end());

        return true;
    }
    // ------
    // ------
#ifdef DREIZEHN_DEBUG
    inline  bool gDumpStateNodes = false; //Debug
    inline  bool gShowVariableDebug = false; //Debug
#endif
}

namespace Tools {
    void PrintParseError(std::string message);
    void PrintRuntimeError(std::string message);
}


} //namespace DreiZehn
