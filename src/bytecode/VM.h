#pragma once
#include <cstring>
#include "CompilerScope.h"
#include "VMStructure.h"
namespace DreiZehn::VM {

    namespace Compiler {
        Value compileExec(Environment& env, ASTNode* node);

        // without attaching functions again
         Value compileCodeletExec(Environment& env, ASTNode* node);

        // when calling directly
        inline BytecodeChunk MasterChunk;

        bool Compile( ASTNode* node, Environment& env, BytecodeChunk& chunk, CompilerScope& scope);
        void prepare(Environment& env, BytecodeChunk& chunk);
        void attachFunctions(Environment& env, BytecodeChunk& chunk);
    }

    Value runDirectThreadedVM(BytecodeChunk& chunk, U32 numLocals, U32 offset = 0);

    // ------------------------------
    // bytecode scope and locals
    // ------------------------------
    inline CompilerScope scope;
    inline std::vector<Value>locals;
    // ------------------------------

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
}
