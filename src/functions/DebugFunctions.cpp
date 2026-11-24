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

#ifdef DREIZEHN_DEBUG
#include <Environment.h>
#include "bytecode/VMStructure.h"
#endif

namespace DreiZehn {


    void RegisterDebugFunctions(Environment& env ) {
         static bool registered = false; if (registered) return; registered = true;
         using namespace FunctionMap;
        // ---------------------------------------------------------------------
         #ifdef DREIZEHN_DEBUG
        RegisterFunction("debug::toggle", [](ValueSpan args, Value& ret) -> bool {
            Globals::gDumpStateNodes = ! Globals::gDumpStateNodes;
            return true;
        });
        #endif
        // ---------------------------------------------------------------------
        // RegisterFunction("debug::printenv", [](ValueSpan args, Value& ret) -> bool {
        //     Tools::printf("Current env: %p\n", (void*)Globals::gCurEnv);
        //     return true;
        // });
        // ---------------------------------------------------------------------
        RegisterFunction("debug::fn", [](ValueSpan args, Value& ret) -> bool {
            Tools::printf("  --- Script Function [%zu] --- \n",RegisteredScriptFunctions.size());
            for (const auto& [key, value] : RegisteredScriptFunctions) {
                Tools::printf("  - %s bodys:%d\n", SymbolTable::getName(key).c_str(), value.body.size());
            }
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("debug::stat", [](ValueSpan args, Value& ret) -> bool {

            Tools::printf("SymbolTable count: %zu\n", SymbolTable::size());
            Tools::printf("StringTable count: %zu\n", StringTable::size());
            Tools::printf("Garbage count    : %zu\n", GarbageCollection::size());
            return true;
        });
        // ---------------------------------------------------------------------
        RegisterFunction("debug::printAst", [&env](ValueSpan args, Value& ret) -> bool {
            #ifdef DREIZEHN_DEBUG
            if (args.size() == 0) {
                Tools::errorf("Usage:debug::printAst filename");
                return false;
            }
            return PrintScriptFileAST(args[0].getString(), env);
            #else
                Tools::warnf("Sorry only available in debug build");
                return true;
            #endif
        }, "Print the AST for the given file. (debug only)");
        // ---------------------------------------------------------------------
        RegisterFunction("debug::printByteCode", [&env](ValueSpan args, Value& ret) -> bool {
            #ifdef DREIZEHN_DEBUG
            if (args.size() > 0) {
                Tools::warnf("Usage:debug::printByteCode");
            }
            VM::PrintChunk(env.getVM().getChunk());
            return true;
            #else
            Tools::warnf("Sorry only available in debug build");
            return true;
            #endif
        }, "Print the current ByteCode. (debug only)");
        // ---------------------------------------------------------------------
        // ---------------------------------------------------------------------
        // DEBUG BYTECODE
        // ---------------------------------------------------------------------
        RegisterFunction("debug::bc", [&env](ValueSpan args, Value& ret) -> bool {

            if (Globals::ByteCodeMode || Globals::ByteCodeRunning) {
                Tools::errorf("Sorry, can't call ByteCode inside bytecode - use include!\n");
                return false;
            }

            if (args.size() != 1 || !args[0].isString()) {
                Tools::errorf("Usage: debug::bc codelet\n");
                ret = NumberNullValue;
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret =  RunScriptStreamByteCode(stream, env);

            return true;
        }, "run a script embedded in a string on BYTECODE VM");

        // Interpreter
        RegisterFunction("debug::ip", [&env](ValueSpan args, Value& ret) -> bool {
            if (args.size() != 1 || !args[0].isString()) {
                Tools::errorf("Usage: debug::bc codelet\n");
                ret = NumberNullValue;
                return true;
            }
            std::stringstream stream(args[0].getStringRef());
            ret =  Value(RunScriptStream(stream, env));

            return true;
        }, "run a script embedded in a string on BYTECODE VM");


    } //RegisterDebugFunctions

} //namespace
