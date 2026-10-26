//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Structure
//-----------------------------------------------------------------------------
#pragma once
#include <vector>
#include <cstdint>
#include <cstring> //mem ...
#include "toolbox/Tools.h"
#include "core/Value.h"

namespace DreiZehn {

struct CallFrame {
    U32 mIP = 0;
    U32 mLocalsStartIndex = 0; // start of current locals
    U32 mLocalsCount = 0;      // locals count
};


#define OP_CODE_LIST \
/* Basic types & stack */ \
X(OP_GET_LOCAL,                1, "get local variable") \
X(OP_POP_STACK,                0, "pop stack top") \
X(OP_CONST,                    1, "push const to stack") \
X(OP_PUSH_VALUE,               0, "push value to stack") \
X(OP_JUMP,                     1, "unconditional jump") \
X(OP_JUMP_CLEAN_STACK,         2, "jump and clean stack") \
X(OP_JUMP_IF_TRUE_NO_POP,      1, "jump if true without popping") \
X(OP_JUMP_IF_FALSE_NO_POP,     1, "jump if false without popping") \
/* ..... */ \
X(OP_JUMP_IF_FALSE,            1, "jump if false") \
X(OP_JUMP_IF_TRUE,             1, "jump if true") \
X(OP_LESS_EQUAL,               0, "compare less equal") \
X(OP_LESS,                     0, "compare less") \
X(OP_GREATER,                  0, "compare greater") \
X(OP_GREATER_EQUAL,            0, "compare greater equal") \
X(OP_EQUAL,                    0, "compare equal") \
X(OP_NOT_EQUAL,                0, "compare not equal") \
/* ..... */ \
X(OP_BIT_AND,                  0, "bitwise AND") \
X(OP_BIT_OR,                   0, "bitwise OR") \
X(OP_BIT_XOR,                  0, "bitwise XOR") \
X(OP_SHL,                      0, "shift left") \
X(OP_SHR,                      0, "shift right") \
X(OP_MODULO,                   0, "modulo operation") \
X(OP_LOAD_VAR,                 3, "load global variable") \
X(OP_SAVE_VAR,                 3, "save global variable") \
/* ..... */ \
X(OP_SET_LOCAL,                1, "set local variable") \
X(OP_LOAD_OBJECT_FIELD,        1 + (2 * 2 /*sizeof(void*)*/), "load object field") \
X(OP_SAVE_OBJECT_FIELD,        1 + (2 * 2 /*sizeof(void*)*/), "save object field") \
X(OP_CALL_OBJECT_METHOD,       3, "call object method") \
X(OP_SAVE_OBJECT_METHOD,       2, "save object method") \
X(OP_LOAD_ARRAY,               0, "load array element") \
X(OP_SAVE_ARRAY,               0, "save array element") \
X(OP_CALL_C_FUNCTION,          0, "SPECIAL emittedCount variable") \
/* ..... */ \
X(OP_CALL_SF_FUNCTION,         1 + ( 2 /*sizeof(void*)*/),  "call script function") \
X(OP_INC_LOCAL,                2, "increment local variable") \
X(OP_ADD_LOCAL,                1, "add to local variable") \
X(OP_SUB_LOCAL,                1, "subtract from local variable") \
X(OP_MUL_LOCAL,                1, "multiply local variable") \
X(OP_DIV_LOCAL,                1, "divide local variable") \
X(OP_ADD,                      0, "addition") \
X(OP_SUB,                      0, "subtraction") \
/* ..... */ \
X(OP_LOOP_INCREMENT_JUMP,      3, "increment loop and jump") \
X(OP_MUL,                      0, "multiplication") \
X(OP_DIV,                      0, "division") \
X(OP_NOT,                      0, "logical NOT") \
X(OP_FOR_RANGE,                2, "for range loop") \
X(OP_FOR_STEP_INIT,            3, "initialize for step loop") \
X(OP_FOR_EACH_INIT,            2, "initialize foreach loop") \
X(OP_FOR_EACH_INCREMENT_JUMP,  2, "increment foreach and jump") \
/* ..... */ \
X(OP_LOAD_OBJECT_FIELD_GENERIC, 1 + (2 * 2 /*sizeof(void*)*/), "load object field with skip cache!") \
X(OP_SAVE_OBJECT_FIELD_GENERIC, 1 + (2 * 2 /*sizeof(void*)*/), "save object field with skip cache!") \
X(OP_CREATE_KEYVALUE,          0, "create key-value pair") \
X(OP_RETURN,                   0, "return from function") \
X(OP_STR_CONCAT,               0, "string add (concat)") \
X(OP_EXIT,                     0, "exit interpreter") \
X(OP_INVALID,                  0, "invalid opcode fallback")


// ----- OpCodes from X Macro -----
#define X(name, args, comment) name,
enum OpCodes : uint32_t {
    OP_CODE_LIST
};
#undef X

// ----- OpCode Infos from X Macro -----
struct OpCodeInfo {
    const char* name;
    U32 emittedCount;
};

#define X(name, args, comment) {#name, args},
constexpr OpCodeInfo OpCodeInfos[] = {
    OP_CODE_LIST
};
#undef X


#ifdef DREIZEHN_DEBUG
void disassembleInstruction(const uint32_t* code, size_t offset, const S32 stk, const size_t callstk, size_t localsSize);
#endif

// -----------------------------------------------------------------------------

// the code chunk
struct BytecodeChunk {
    std::vector<U32> mByteCodes;
    std::vector<Value> mConstants;

    U32 addConstant(Value val) {
        mConstants.push_back(val);
        return static_cast<U32>(mConstants.size() - 1);
    }


    // U32 emitValue(Value val) {
    //     U32 startIp = (U32)mByteCodes.size();
    //     mByteCodes.resize(startIp + 2);
    //     std::memcpy(&mByteCodes[startIp], &val, sizeof(Value));
    //     return startIp;
    // }
    U32 emitValue(Value val) {
        static_assert(sizeof(Value) <= 2 * sizeof(U32), "Value size missmatch! Only 8 bytes allowed.");

        U32 startIp = (U32)mByteCodes.size();
        mByteCodes.resize(startIp + 2);

        if (sizeof(Value) < 8) {
            mByteCodes[startIp] = 0;
            mByteCodes[startIp + 1] = 0;
        }

        std::memcpy(&mByteCodes[startIp], &val, sizeof(Value));
        return startIp;
    }

    // U32 emitPointer(void* ptr) {
    //     U32 startIp = (U32)mByteCodes.size();
    //     mByteCodes.resize(startIp + 2);
    //
    //     std::memcpy(&mByteCodes[startIp], &ptr, sizeof(void*));
    //
    //     return startIp;
    // }
    U32 emitPointer(void* ptr) {
        U32 startIp = (U32)mByteCodes.size();
        size_t slotsNeeded = sizeof(void*) / sizeof(U32);
        mByteCodes.resize(startIp + slotsNeeded);
        std::memcpy(&mByteCodes[startIp], &ptr, sizeof(void*));
        return startIp;
    }


    U32 tell() {
        return (U32)mByteCodes.size();
    }

    bool patch(U32 pos, U32 code) {
        if (pos >= mByteCodes.size()) return false;
        mByteCodes[pos] = code;
        return true;
    }

    bool patchPtr(U32 pos, void* ptr) {
        if ((pos + 2) >= mByteCodes.size()) return false;
        std::memcpy(&mByteCodes[pos], &ptr, sizeof(void*));
        return true;
    }

    U32 emitS32 (S32 op) {
        mByteCodes.push_back(static_cast<U32>(op));
        return (U32)mByteCodes.size() - 1;
    }

    U32 emitF32 (F32 op) {
        mByteCodes.push_back(static_cast<U32>(op));
        return (U32)mByteCodes.size() - 1;
    }


    U32 emit(U32 op) {
        mByteCodes.push_back(op);
        return (U32)mByteCodes.size() - 1;
    }

};

} // namespace DreiZehn
