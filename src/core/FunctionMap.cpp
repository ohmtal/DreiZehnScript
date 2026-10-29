//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Value Object
//-----------------------------------------------------------------------------
#include <vector>

#include <string.h>

#include "Value.h"
#include "FunctionMap.h"
#include "Environment.h"
#include "bytecode/VM.h"

namespace DreiZehn::FunctionMap {
    // -------------------------------------------------------------------------
    Value CallScriptFunction(Environment& env,const ScriptFunction* sf, std::span<Value> args)
    {
        if (sf) {
            auto& func = *(sf);
            Environment localEnv(&env);

            for (size_t i = 0; i < func.parameterNames.size(); ++i) {
                if (i < args.size()) {
                    localEnv.getVariableFrame()->setVariable(
                        SymbolTable::insert(func.parameterNames[i])
                        , args[i]
                        , true);
                } else {
                    // 0.8d fill up arguments with 0
                    localEnv.getVariableFrame()->setVariable(SymbolTable::insert(func.parameterNames[i]), NumberNullValue, true);
                }
            }

            for (auto& statement : func.body) {
                FlowStatus status= env.execute(statement.get(), localEnv);

                if (status.signal == FlowSignal::Return) {
                    return status.returnValue;
                }
            }
            return NumberNullValue;
        }
        return NumberNullValue;
    }
    // -------------------------------------------------------------------------
    // Events
    // -------------------------------------------------------------------------
    void RegisterScriptEvent( Environment* envPtr, ScriptEvent& event,
                              const std::string& funcName, const std::vector<std::string>& parameters
                              , const std::string& desc )
    {
        if (!envPtr) return;
        event.envPtr = envPtr;
        event.sf =  DreiZehn::FunctionMap::RegisterScriptFunctionStub(funcName,parameters);
        event.desc = desc;
    }

    // -------------------------------------------------------------------------
    Value FireScriptEvent( ScriptEvent& event, std::vector<Value> arguments) {
        if (!event.sf || !event.envPtr) {
            Tools::errorf("FireScriptEvent - Init first - function not found!");
            return NumberNullValue;
        }
        if ( Globals::ByteCodeMode ) {

            // Copy ! :/
            BytecodeChunk chunk =  DreiZehn::VM::Compiler::MasterChunk;
            // // DreiZehn::VM::Compiler::prepare(*event.envPtr,chunk);

            size_t chunkArguments = arguments.size();
            uint32_t ip = chunk.tell();
            // arguments
            for (auto& arg: arguments) {
                uint32_t constIdx = chunk.addConstant(arg);
                chunk.addConstant(constIdx);
                chunk.emit(OP_CONST);
                chunk.emit(constIdx);
            }

            // main function
            chunk.emit(OP_CALL_SF_FUNCTION);
            chunk.emit((U32)chunkArguments);
            chunk.emitPointer(event.sf);
            // chunk.emit(OP_POP_STACK); //?? need the result
            chunk.emit(OP_EXIT);


            return VM::runDirectThreadedVM(chunk, VM::scope.getLocalCount(), ip);

        } else {
            return FunctionMap::CallScriptFunction(*event.envPtr, event.sf, arguments);
        }
        return NumberNullValue;
    }
    // -------------------------------------------------------------------------
    //
    // -------------------------------------------------------------------------

}
