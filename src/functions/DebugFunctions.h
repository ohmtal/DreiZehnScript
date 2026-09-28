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


    void RegisterDebugFunctions( ) {
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
        RegisterFunction("debug::stat", [](std::vector<Value>& args, Value& ret) -> bool {

            Tools::printf("SymbolTable count: %zu\n", SymbolTable::size());
            Tools::printf("StringTable count: %zu\n", StringTable::size());
            Tools::printf("Garbage count    : %zu\n", gMasterFrame->getGarbageSize());
            return true;
        });

    } //RegisterDebugFunctions

} //namespace
