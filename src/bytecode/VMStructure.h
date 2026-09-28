//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Structure
//-----------------------------------------------------------------------------
#pragma once
#pragma once
#include <vector>
#include <cstdint>
#include <cstring> //mem ...

namespace DreiZehn {

typedef uint32_t U32; // dont want to break my fingers lol
typedef int32_t  S32; // dont want to break my fingers lol

enum  OpCodes : uint32_t {
    OP_CONST,       // push const to stack

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_LESS_EQUAL,

    OP_LOAD_VAR,
    OP_SAVE_VAR,

    // for future use ...
    OP_GET_LOCAL,   // fetch var
    OP_SET_LOCAL,   // write var

    // Math
    OP_INC_LOCAL, //++ -- += -= using emitted value
    // // OP_DEC_LOCAL, //--  <<  use inc local with emitted neg value!
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_EXIT,        // bye bye
    OP_INVALID
};

struct OpCodeInfo {
  const char* name;
  U32 emittedCount;
};

inline OpCodeInfo OpCodeInfos[] = {
    {"OP_CONST", 1},

    {"OP_JUMP", 0},
    {"OP_JUMP_IF_FALSE", 0},

    {"OP_LESS_EQUAL", 0},

    {"OP_LOAD_VAR", 1},
    {"OP_SAVE_VAR", 1},

    {"OP_GET_LOCAL", 1},
    {"OP_SET_LOCAL", 1},

    {"OP_INC_LOCAL", 2},
    // // "OP_DEC_LOCAL" use inc local with emitted neg value!

    {"OP_ADD", 0},
    {"OP_SUB", 0},
    {"OP_MUL", 0},
    {"OP_DIV", 0},

    {"OP_EXIT", 0},
    {"OP_INVALID", 0}
};



inline void disassembleInstruction(const uint32_t* code, size_t offset) {
    uint32_t opcode = code[offset];

    Tools::printf("%04zu: ", offset);

    if (opcode < OP_INVALID) {
        Tools::printf("%s", OpCodeInfos[opcode].name);
        if (OpCodeInfos[opcode].emittedCount > 0) {
            Tools::printf(" Args: ");
            for (U32 i = 1; i <= OpCodeInfos[opcode].emittedCount; i++ ) {
                Tools::printf("%u ", code[offset + i]);
            }
        }
    } else {
        Tools::printf("UNKNOWN_OPCODE [%u]", opcode);
    }
    Tools::printf("\n");
}


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

    U32 emit(U32 op) {
        mByteCodes.push_back(op);
        return (U32)mByteCodes.size() - 1;
    }

};

} // namespace DreiZehn
