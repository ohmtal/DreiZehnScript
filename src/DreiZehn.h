//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Glue and Module Registry / import
//-----------------------------------------------------------------------------
#pragma once

#include "Environment.h"

#include "functions/CoreFunctions.h"
#include "functions/StringFunctions.h"
#include "functions/MathFunctions.h"
#include "functions/VectorFunctions.h"
#include "functions/DebugFunctions.h"
#include "functions/PointVectorObjectFunctions.h"


namespace DreiZehn {

void Init(Environment& env) {
    static bool registered = false; if (registered) return; registered = true;

    using namespace FunctionMap;
    ModuleRegistry::Register("Core", RegisterCoreFunctions);
    ModuleRegistry::Register("Math", RegisterMathFunctions);
    ModuleRegistry::Register("String", RegisterStringFunctions);
    ModuleRegistry::Register("Vector", RegisterVectorFunctions);
    ModuleRegistry::Register("Debug", RegisterDebugFunctions);
    ModuleRegistry::Register("PointVector", RegisterPointVectorObjectFunctions);


    // ---------------------------------------------------------------------
    // also push some CORE Constants here:
    RegisterConstants("true", Value(1));
    RegisterConstants("false", Value(0));
    // ---------------------------------------------------------------------

    // ---------------------------------------------------------------------
    // -------- print --------------
    // ---------------------------------------------------------------------
    RegisterFunction("print", [](std::vector<Value>& args, Value& ret) -> bool {
        std::string line = "";
        for ( auto& value : args) {
            line = line + value.toString() + " ";
        }
        Tools::printf("%s\n", line.c_str());
        return true;
    });
    RegisterFunction("printraw", [](std::vector<Value>& args, Value& ret) -> bool {
        for ( auto& value : args) {
            value.print();
        }
        return true;
    });

    RegisterFunction("error", [](std::vector<Value>& args, Value& ret) -> bool {
        std::string line = "";
        for ( auto& value : args) {
            line = line + value.toString() + " ";
        }
        Tools::errorf("%s\n", line.c_str());
        return true;
    });
    // ---------------------------------------------------------------------
    // run
    // ---------------------------------------------------------------------
    RegisterFunction("run", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("file name requires for run, Usage: run filename\n");
            ret = Value(0);
            return true;
        }

        if (args[0].isStringId() ) {
            Tools::printf("Loading Script: %s\n", args[0].getStringRef().c_str());
            bool success = RunScriptFile(args[0].getStringRef(), env);
            ret = Value(success ? 1 : 0);
            return success;
        }


        Tools::errorf("file name requires for run\n");
        ret = Value(0);
        return true;
    });

    // ---------------------------------------------------------------------
    // list modules
    // ---------------------------------------------------------------------
    RegisterFunction("modules", [&env](std::vector<Value>& args, Value& ret) -> bool {
        ModuleRegistry::Print();
        return true;
    });
    // ---------------------------------------------------------------------
    // int
    // ---------------------------------------------------------------------
    RegisterFunction("int::cast", [](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("Usage: int::cast value\n");
            return false;
        }
        if (args[0].isInt()) ret =  args[0];
        else
            if (args[0].isDouble()) ret = Value(args[0].getInt());
            else
                if (args[0].isStringId()) ret = Value( std::atoi( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to integer!\n");

        return true;
    });


    // ---------------------------------------------------------------------
    // float
    // ---------------------------------------------------------------------
    RegisterFunction("float::cast", [](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() != 1) {
            Tools::errorf("Usage: float::cast value\n");
            return false;
        }
        if (args[0].isDouble()) ret =  args[0];
        else
            if (args[0].isInt()) ret = Value(args[0].getDouble());
            else
                if (args[0].isStringId()) ret = Value( std::atof( args[0].getStringRef().c_str()));
                else
                    Tools::PrintRuntimeError("Cant cast pointer to float!\n");

        return true;
    });

}

} //namespace

