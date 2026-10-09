//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Core Commands
//-----------------------------------------------------------------------------
#pragma once
#include "core/FunctionMap.h"
#include "Globals.h"

namespace DreiZehn {


    void RegisterDebugFunctions(Environment& env ) {
         using namespace FunctionMap;
        // ---------------------------------------------------------------------
        RegisterFunction("debug::toggle", [](std::vector<Value>& args, Value& ret) -> bool {
            Globals::gDumpStateNodes = ! Globals::gDumpStateNodes;
            return true;
        });
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

    } //RegisterDebugFunctions

} //namespace
