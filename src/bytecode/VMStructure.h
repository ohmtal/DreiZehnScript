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

// dont want to break my fingers every time
typedef uint32_t U32;
typedef int64_t  S64;
typedef int32_t  S32;

struct CallStack {
    U32 mIP = 0;
    std::vector<Value> mLocals;
};

enum  OpCodes : uint32_t {
    OP_CONST,       // push const to stack
    OP_PUSH_VALUE,

    OP_JUMP,
    OP_JUMP_CLEAN_STACK,
    OP_JUMP_IF_TRUE_NO_POP,
    OP_JUMP_IF_FALSE_NO_POP,
    OP_JUMP_IF_FALSE,

    OP_LESS_EQUAL,
    OP_LESS,

    OP_GREATER,
    OP_GREATER_EQUAL,
    OP_EQUAL,
    OP_NOT_EQUAL,

    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_SHL,
    OP_SHR,
    OP_MODULO,



    // Global Variables
    OP_LOAD_VAR,
    OP_SAVE_VAR,

    // Local Variables:
    OP_GET_LOCAL,
    OP_SET_LOCAL,

    // Object fields
    OP_LOAD_OBJECT_FIELD,
    OP_SAVE_OBJECT_FIELD,

    // Object methods
    OP_CALL_OBJECT_METHOD,
    OP_SAVE_OBJECT_METHOD,

    //Function Call
    OP_CALL_C_FUNCTION,
    OP_CALL_SF_FUNCTION,


    // Math
    OP_INC_LOCAL, //++ -- += -= using emitted value

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,



    // Loop
    OP_FOR_RANGE,
    OP_FOR_STEP_INIT,
    OP_LOOP_INCREMENT_JUMP,


    OP_POP_STACK,

    OP_RETURN,
    OP_EXIT,        // bye bye
    OP_INVALID
};

struct OpCodeInfo {
  const char* name;
  U32 emittedCount;
};

constexpr OpCodeInfo OpCodeInfos[] = {
    {"OP_CONST", 1},
    {"OP_PUSH_VALUE", 0},

    {"OP_JUMP", 1},
    {"OP_JUMP_CLEAN_STACK", 2},
    {"OP_JUMP_IF_TRUE_NO_POP", 1},
    {"OP_JUMP_IF_FALSE_NO_POP", 1},
    {"OP_JUMP_IF_FALSE", 1},

    {"OP_LESS_EQUAL", 0},
    {"OP_LESS", 0},

    {"OP_GREATER", 0 },
    {"OP_GREATER_EQUAL", 0 },
    {"OP_EQUAL", 0 },
    {"OP_NOT_EQUAL", 0 },

    {"OP_BIT_AND", 0 },
    {"OP_BIT_OR", 0 },
    {"OP_BIT_XOR", 0 },
    {"OP_SHL", 0 },
    {"OP_SHR", 0 },
    {"OP_MODULO", 0 },


    {"OP_LOAD_VAR", 3},
    {"OP_SAVE_VAR", 3},

    {"OP_GET_LOCAL", 1},
    {"OP_SET_LOCAL", 1},

    {"OP_LOAD_OBJECT_FIELD", 5},
    {"OP_SAVE_OBJECT_FIELD", 5},

    {"OP_CALL_OBJECT_METHOD", 2},
    {"OP_SAVE_OBJECT_METHOD", 2},

    {"OP_CALL_C_FUNCTION", 0}, //SPECIAL emittedCount variable
    {"OP_CALL_SF_FUNCTION", 2},

    {"OP_INC_LOCAL", 2},

    {"OP_ADD", 0},
    {"OP_SUB", 0},
    {"OP_MUL", 0},
    {"OP_DIV", 0},


    {"OP_FOR_RANGE", 2},
    {"OP_FOR_STEP_INIT", 3 },
    {"OP_LOOP_INCREMENT_JUMP", 3},

    {"OP_POP_STACK", 0},

    {"OP_RETURN", 0},
    {"OP_EXIT", 0},
    {"OP_INVALID", 0}
};



void disassembleInstruction(const uint32_t* code, size_t offset, const S32 stk, const size_t callstk, size_t localsSize);


// -----------------------------------------------------------------------------

// the code chunk
struct BytecodeChunk {
    std::vector<U32> mByteCodes;
    std::vector<Value> mConstants;

    U32 addConstant(Value val) {
        mConstants.push_back(val);
        return static_cast<U32>(mConstants.size() - 1);
    }


    U32 emitValue(Value val) {
        U32 startIp = (U32)mByteCodes.size();
        mByteCodes.resize(startIp + 2);
        std::memcpy(&mByteCodes[startIp], &val, sizeof(Value));
        return startIp;
    }

    U32 emitPointer(void* ptr) {
        U32 startIp = (U32)mByteCodes.size();
        mByteCodes.resize(startIp + 2);

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
        if ((pos + sizeof(void*)) >= mByteCodes.size()) return false;
        std::memcpy(&mByteCodes[pos], &ptr, sizeof(void*));
        return true;
    }

    U32 emitS32 (S32 op) {
        mByteCodes.push_back((U32)op);
        return (U32)mByteCodes.size() - 1;
    }

    U32 emit(U32 op) {
        mByteCodes.push_back(op);
        return (U32)mByteCodes.size() - 1;
    }

};

} // namespace DreiZehn
