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

    inline std::vector<uint32_t> GlobalVarList;
    inline bool isGlobalVar(uint32_t id) {
          auto it = std::find(GlobalVarList.begin(), GlobalVarList.end(), id);
          if (it == GlobalVarList.end()) return false;
          return true;
    }
    inline bool insertGlobalVar(uint32_t id) {
        if (isGlobalVar(id)) return false;
        GlobalVarList.push_back(id);
        return true;
    }
    // ------
    // ------
    inline  bool gDumpStateNodes = false; //Debug
    inline  bool gShowVariableDebug = false; //Debug

}

namespace Tools {
    inline void PrintParseError(std::string message) {
        errorf("Parse Error in Line [%d]\n%s\n%s\n",
            Globals::currentScriptLineNumber,
            Globals::currentScriptLine.c_str(),
            message.c_str());
    }

    inline void PrintRuntimeError(std::string message) {
        errorf("RunTime Error: %s", message.c_str());
    }
}


} //namespace DreiZehn
