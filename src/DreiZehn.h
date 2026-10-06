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
    using namespace FunctionMap;
    ModuleRegistry::Register("Core", RegisterCoreFunctions);
    ModuleRegistry::Register("Math", RegisterMathFunctions);
    ModuleRegistry::Register("String", RegisterStringFunctions);
    ModuleRegistry::Register("Vector", RegisterVectorFunctions);
    ModuleRegistry::Register("Debug", RegisterDebugFunctions);
    ModuleRegistry::Register("PointVector", RegisterPointVectorObjectFunctions);

    RegisterFunction("modules", [&env](std::vector<Value>& args, Value& ret) -> bool {
        ModuleRegistry::Print();
        return true;
    });

    // FIXME move import to lexer and call it when it's found there !!
    RegisterFunction("import", [&env](std::vector<Value>& args, Value& ret) -> bool {
        if (args.size() == 0) {
            Tools::errorf("Usage example: import Module::Core Module::Math...");
            return false;
        }


        ret = Value(1);

        for (auto& arg: args) {
            if (arg.isStringId()) {
                std::string str = arg.getStringRef();
                if (str.compare("Module::All") == 0 || str.compare("All") == 0) {
                    ret = Value(ModuleRegistry::LoadAll(env));
                    return true;
                } else {
                    uint32_t lookupId = 0;
                    if (!Tools::begins_with(str,"Module::")) str = "Module::" + str;
                    lookupId = SymbolTable::insert(str);
                    if (!ModuleRegistry::Load(lookupId,env)) {
                        ret = Value(0);
                        Tools::errorf("Failed to load module named: %s\n", str.c_str());
                    }
                }
            } else if (arg.isNumber()) {
                uint32_t id = arg.getUInt();
                if (!ModuleRegistry::Load(id,env)) {
                    ret = Value(0);
                    Tools::errorf("Failed to load module %d\n", id);
                }
            }
        }


        return true;
    });

}

} //namespace
