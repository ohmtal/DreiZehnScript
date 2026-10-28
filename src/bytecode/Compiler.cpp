//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "Environment.h"
#include "bytecode/VM.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/VMStructure.h"


namespace DreiZehn::VM::Compiler {

   void patch(BytecodeChunk& chunk) {
        // Apply function patches - what a HACK fest
        // Love this part :D
        for (auto& patch: mFunctionPatches) {
            if (patch.mPatchType == FuncPatchType::Adress)
                chunk.patch(patch.mPatchByteCodePos, static_cast<FunctionMap::ScriptFunction*>(patch.mFuncPtr)->bytecodeAddress);
            else if (patch.mPatchType == FuncPatchType::VarCount)
                chunk.patch(patch.mPatchByteCodePos, static_cast<FunctionMap::ScriptFunction*>(patch.mFuncPtr)->localVarCount);
            else
            // new 0.9f
            if (patch.mPatchType == FuncPatchType::ScriptFuncPtr)
                chunk.patchPtr(patch.mPatchByteCodePos, patch.mFuncPtr);
            else if (patch.mPatchType == FuncPatchType::CFuncPrt)
                chunk.patchPtr(patch.mPatchByteCodePos, patch.mFuncPtr);
            else
                assert(true && "FunctionPatches unknown PathType!!!");
        }
        mFunctionPatches.clear();
    }

    // -------------------------------------------------------------------------
    void attachFunctions(Environment& env, BytecodeChunk& chunk) {
        // ----------------- ATTACH FUNCTIONS ----------------------------
        ///  added scripts on back of byte code after EXIT
        // LOL ON THE BACK I DONT HAVE THE **bytecodeAddress** on a function call!
        // NOTE the function bytecode and functScope should be cached at script function!
        //      with a dirty flag -
        for (auto& [symbolId, func] : FunctionMap::RegisteredScriptFunctions) {
            func.bytecodeAddress = chunk.tell();

            CompilerScope funcScope;

            // ----
            func.compiledConstantsOffset = chunk.mConstants.size();
            func.constantsOffset = 0;
            // ----

            for (const auto& paramName : func.parameterNames) {
                funcScope.insert(SymbolTable::insert(paramName));
            }

            for (auto& statement : func.body) {
                statement->compile(env, chunk, funcScope, ValueType::NONE);
            }
            // -----

            // // if (func.bytecodeDirty) {
            // //     func.compiledConstantsOffset = chunk.mConstants.size();
            // //     func.constantsOffset = 0;
            // //
            // //     for (const auto& paramName : func.parameterNames) {
            // //         funcScope.insert(SymbolTable::insert(paramName));
            // //     }
            // //
            // //     for (auto& statement : func.body) {
            // //         statement->compile(env, chunk, funcScope, ValueType::NONE);
            // //     }
            // //
            // //     // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
            // //     // fixed: cache does not work because of constants index missmatch!
            // //     // FIXME: cache does not work because of absolute jumps in many Expressions !!!
            // //     /*
            // //
            // //     // Cache it ....
            // //     func.scope = funcScope;
            // //     func.chunk.mByteCodes.assign(
            // //         chunk.mByteCodes.begin() + func.bytecodeAddress ,
            // //         chunk.mByteCodes.end()
            // //     );
            // //     if (chunk.mConstants.size() > func.compiledConstantsOffset) {
            // //         func.chunk.mConstants.assign( chunk.mConstants.begin() + func.compiledConstantsOffset , chunk.mConstants.end()
            // //         );
            // //     } else {
            // //         func.chunk.mConstants.clear();
            // //     }
            // //
            // //     // func.bytecodeDirty = false;
            // //     */
            // //
            // // } else {
            // //     // Use the cache
            // //     // append the func.chunk.mByteCodes func.chunk.mConstants to chunk
            // //
            // //     funcScope = func.scope;
            // //
            // //     chunk.mByteCodes.insert(
            // //             chunk.mByteCodes.end(),
            // //             func.chunk.mByteCodes.begin(),
            // //             func.chunk.mByteCodes.end()
            // //     );
            // //
            // //     size_t currentConstantPos = chunk.mConstants.size();
            // //     func.constantsOffset  = currentConstantPos - func.compiledConstantsOffset ;
            // //
            // //     chunk.mConstants.insert(
            // //             chunk.mConstants.end(),
            // //             func.chunk.mConstants.begin(),
            // //             func.chunk.mConstants.end()
            // //     );
            // // }

            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(NumberNullValue);
            chunk.emit(OP_RETURN);

            func.localVarCount = funcScope.getLocalCount();
        }
        patch(chunk);
    }
    // -------------------------------------------------------------------------
    void prepare(Environment& env, BytecodeChunk& chunk) {

        mFunctionPatches.clear();
    }
    // -------------------------------------------------------------------------
    bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope)
    {
        if (!node) return false;

        prepare(env,chunk);

        switch(node->mNodeType) {
            case NodeType::BlockStatement:
            {
                auto* flowStmt = static_cast<BlockStatement*>(node);
                if (!flowStmt->compile(env, chunk, scope, ValueType::NONE)) return false;
                break;
            }
            default:
                Tools::errorf("UNKNOWN Expression Compile: %s\n", NodeTypeToString(node->mNodeType));
                return false;
        }


        chunk.emit(OP_EXIT);

        attachFunctions(env,chunk);

        return true;
    }
    // -------------------------------------------------------------------------
    Value compileExec(Environment& env, ASTNode* node) {
        if (!node) return NumberNullValue;

        BytecodeChunk chunk;


        // CompilerScope scope;
        if (!Compile(node, env, chunk, VM::scope)) {
            Tools::errorf("Failed to compile code!\n");
            return NumberNullValue;
        }
        // after Compile we patch :)
        // moved to attachFunctions==> Patch(chunk);

        Value res = VM::runDirectThreadedVM(chunk, VM::scope.getLocalCount());
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** CompileExec VALUE IS: %s\n", res.toString().c_str());
        #endif
        return res;
    }
}
