#include "Environment.h"
#include "bytecode/VM.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/VMStructure.h"


namespace DreiZehn {

   void Patch(BytecodeChunk& chunk) {
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

    bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope)
    {
        if (!node) return false;

        mFunctionPatches.clear();

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

        ///  added scripts on back of byte code after EXIT
        // LOL ON THE BACK I DONT HAVE THE **bytecodeAddress** on a function call!
        for (auto& [symbolId, func] : FunctionMap::RegisteredScriptFunctions) {
            func.bytecodeAddress = chunk.tell();

            CompilerScope funcScope;

            for (const auto& paramName : func.parameterNames) {
                funcScope.insert(SymbolTable::insert(paramName));
            }

            for (auto& statement : func.body) {
                statement->compile(env, chunk, funcScope, ValueType::NONE);
            }

            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(NumberNullValue);
            chunk.emit(OP_RETURN);

            func.localVarCount = funcScope.getLocalCount();
        }



        return true;
    }

    Value compileExec(Environment& env, ASTNode* node) {
        if (!node) return NumberNullValue;
        BytecodeChunk chunk;
        CompilerScope scope;
        if (!Compile(node, env, chunk, scope)) {
            Tools::errorf("Failed to compile code!\n");
            return NumberNullValue;
        }
        // after Compile we patch :)
        Patch(chunk);

        Value res = runDirectThreadedVM(chunk, scope.getLocalCount());
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** CompileExec VALUE IS: %s\n", res.toString().c_str());
        #endif
        return res;
    }
}
