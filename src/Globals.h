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

    const double EPSILON = 1e-9;



namespace Globals {
    inline int currentScriptLineNumber = 0;
    inline std::string currentScriptLine =  "";
    // ------
    inline  bool shutDownRUNNING = false;
    // ------
    // i want to track the current Env!
    inline Environment* gCurEnv = nullptr;
    inline Environment* gMainEnv = nullptr;

    inline VariableFrame* MasterVariableFrame = nullptr;

    // as sorted list for fast binary_search
    inline std::vector<uint32_t> GlobalVarList;
    inline bool isGlobalVar(uint32_t id) {
          return std::binary_search(GlobalVarList.begin(), GlobalVarList.end(), id);
          // auto it = std::find(GlobalVarList.begin(), GlobalVarList.end(), id);
          // if (it == GlobalVarList.end()) return false;
          // return true;
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
    inline  bool gDumpStateNodes = false; //Debug
    inline  bool gShowVariableDebug = false; //Debug

}

namespace Tools {
    inline void PrintParseError(std::string message) {
        errorf("DreiZehn Parse Error in Line [%d]:%s\n\t%s\n",
            Globals::currentScriptLineNumber,
            Globals::currentScriptLine.c_str(),
            message.c_str());
    }

    inline void PrintRuntimeError(std::string message) {
        errorf("RunTime Error: %s", message.c_str());
    }
}


} //namespace DreiZehn
