//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "Environment.h"
#include "bytecode/VM.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/VMStructure.h"


namespace DreiZehn {

    // bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope);
    // -------------------------------------------------------------------------
    bool VM::Compile( ASTNode* node, Environment& env
        , BytecodeChunk& chunk, CompilerScope& scope)
    {
        if (!node) return false;

        switch(node->mNodeType) {
            case NodeType::MainBlock:
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


        return true;
    }
    // -------------------------------------------------------------------------
    Value VM::compileExec(Environment& env, ASTNode* node) {
        if (!node) return NumberNullValue;

        BytecodeChunk& chunk = mChunk;
        U32 offset = chunk.tell();

        // CompilerScope scope;
        if (!Compile(node, env, chunk, mCompilerScope)) {
            Tools::errorf("Failed to compile code!\n");
            return NumberNullValue;
        }
        // after Compile we patch :)
        // moved to attachFunctions==> Patch(chunk);

        // Value res = VM::runVM(env, chunk,mCompilerScope.getLocalCount(), offset);
        Value res = runVM(env, chunk,mCompilerScope.getLocalCount(), offset);
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** CompileExec VALUE IS: %s\n", res.toString().c_str());
        #endif
        return res;
    }
}
