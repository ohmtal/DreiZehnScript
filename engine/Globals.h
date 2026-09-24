//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
#pragma once

#include "toolbox/Tools.h"

namespace DreiZehn {
    class Environment;

    const double EPSILON = 1e-9;


namespace Globals {
    inline int currentScriptLineNumber = 0;
    inline std::string currentScriptLine =  "";

    // i want to track the current Env!
    inline Environment* gCurEnv = nullptr;
    // inline const std::string gEmptyString = ""; unused!
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
