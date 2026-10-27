//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Glue and Module Registry / import
//-----------------------------------------------------------------------------
#pragma once

#include "Environment.h"

#include "Globals.h"
// #include "core/BaseObjects.h"
// #include "functions/StringFunctions.h"
// #include "functions/MathFunctions.h"
// #include "functions/VectorFunctions.h"
// #include "functions/PointVectorObjectFunctions.h"
// #include "functions/FileFunctions.h"
// #include "functions/SysFunctions.h"


namespace DreiZehn {


    void RegisterCoreFunctions( Environment& env);
    void RegisterDebugFunctions(Environment& env );
    void RegisterMathFunctions(Environment& env);
    void RegisterStringFunctions(Environment& env );
    void RegisterSysFunctions(Environment& env);


    void Init(Environment& env, int argc, char* argv[]);

    namespace Core {
        Value Eval(const std::string& command, Environment& env);
        bool IsFunction(const std::string& funcName) ;
    }


} //namespace

