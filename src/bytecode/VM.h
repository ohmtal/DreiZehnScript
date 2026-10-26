#pragma once
#include <cstring>
namespace DreiZehn {

    Value compileExec(Environment& env, ASTNode* node);

    Value runDirectThreadedVM(BytecodeChunk& chunk, U32 numLocals);

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
