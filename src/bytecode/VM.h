//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//--------------------------------------------------------------------------
// NOTE changed from global functions to class but still hace the redundant
//      variables
//--------------------------------------------------------------------------


#pragma once
#include <cstring>
#include "CompilerScope.h"
#include "VMStructure.h"


namespace DreiZehn {



    class VM {

        BytecodeChunk mChunk;
        CompilerScope mCompilerScope;

        // NOTE:
        // moving the locals to class cause slower test_var/test_struct !
        // Outside inline is not a option
        // inside function initialied with 64 is fast like before but can access them on next
        // run.
        ValueVector mLocals;

    public:
        VM() {
            // mLocals.resize(LOCALS_SIZE);
        }
        Value compileExec(Environment& env, ASTNode* node);
        bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope);

        Value runVM(Environment& env, BytecodeChunk& chunk, U32 numLocals, U32 offset = 0);

        inline void resetChunk() { mChunk.reset(); }
        inline BytecodeChunk& getChunk() { return mChunk; }
        inline CompilerScope& getScope() { return mCompilerScope; }

    private:
        inline Value advance_value(const U32* code, U32& ip) {
            Value val;
            std::memcpy(&val, &code[ip], sizeof(Value));
            ip += 2;
            return val;
        }


        inline void* advance_ptr(const U32* code, U32& ip) {
            void* ptr;
            std::memcpy(&ptr, &code[ip], sizeof(void*));
            ip += 2;
            return ptr;
        }

        inline void patch_ptr(U32* code, U32 pos, void* ptr) {
            code[pos] = 0;
            code[pos + 1] = 0;
            std::memcpy(&code[pos], &ptr, sizeof(void*));
        }

        inline void _set_var_pointer_value(Value* pre, Value& post) {
            if (pre->isObjectPointer())  pre->asPointerObject()->setAssigned(false);
            if (post.isObjectPointer()) post.asPointerObject()->setAssigned(true);
            *pre = post;
        }

    };



}
