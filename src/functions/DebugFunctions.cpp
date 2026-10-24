//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#include <sstream>
#include "core/FunctionMap.h"
#include "Globals.h"
#include <core/GarbageCollection.h>
#include <ScriptLoader.h>

namespace DreiZehn {


    void RegisterDebugFunctions(Environment& env ) {
         static bool registered = false; if (registered) return; registered = true;
         using namespace FunctionMap;
        // ---------------------------------------------------------------------
         #ifdef DREIZEHN_DEBUG
        RegisterFunction("debug::toggle", [](std::vector<Value>& args, Value& ret) -> bool {
            Globals::gDumpStateNodes = ! Globals::gDumpStateNodes;
            return true;
        });
        #endif
        // ---------------------------------------------------------------------
        RegisterFunction("debug::printenv", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("Current env: %p\n", (void*)Globals::gCurEnv);
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("debug::fn", [](std::vector<Value>& args, Value& ret) -> bool {
            Tools::printf("  --- Script Function [%zu] --- \n",RegisteredScriptFunctions.size());
            for (const auto& [key, value] : RegisteredScriptFunctions) {
                Tools::printf("  - %s bodys:%d\n", SymbolTable::getName(key).c_str(), value.body.size());
            }
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("debug::stat", [](std::vector<Value>& args, Value& ret) -> bool {

            Tools::printf("SymbolTable count: %zu\n", SymbolTable::size());
            Tools::printf("StringTable count: %zu\n", StringTable::size());
            Tools::printf("Garbage count    : %zu\n", GarbageCollection::size());
            return true;
        });
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        // DEBUG BYTECODE
        // ---------------------------------------------------------------------
        RegisterFunction("debug::bc", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isString()) {
                Tools::errorf("Usage: debug::bc codeled\n");
                ret = NumberNullValue;
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret =  RunScriptStreamByteCode(stream, env);

            return true;
        }, "run a script embedded in a string on BYTECODE VM");

        // Interpreter
        RegisterFunction("debug::ip", [&env](std::vector<Value>& args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isString()) {
                Tools::errorf("Usage: debug::bc codeled\n");
                ret = NumberNullValue;
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret =  Value(RunScriptStream(stream, env));

            return true;
        }, "run a script embedded in a string on BYTECODE VM");


    } //RegisterDebugFunctions

} //namespace
