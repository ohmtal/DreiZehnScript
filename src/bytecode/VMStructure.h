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
#include "toolbox/Tools.h"

namespace DreiZehn {

// dont want to break my fingers every time
typedef uint32_t U32;
typedef int64_t  S64;
typedef int32_t  S32;

enum  OpCodes : uint32_t {
    OP_CONST,       // push const to stack
    OP_PUSH_VALUE,

    OP_JUMP,
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

    //Function Call
    OP_CALL_C_FUNCTION,
    OP_CALL_SF_FUNCTION,

    // Math
    OP_INC_LOCAL, //++ -- += -= using emitted value
    OP_INC_PTR,   // HACK fest hybrid

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,



    // Loop
    OP_FOR_RANGE,
    OP_FOR_STEP_INIT,
    OP_LOOP_INCREMENT_JUMP,


    OP_POP_STACK,
    OP_EXIT,        // bye bye
    OP_INVALID
};

struct OpCodeInfo {
  const char* name;
  U32 emittedCount;
};

inline OpCodeInfo OpCodeInfos[] = {
    {"OP_CONST", 1},
    {"OP_PUSH_VALUE", 0},

    {"OP_JUMP", 1},
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


    {"OP_LOAD_VAR", 1},
    {"OP_SAVE_VAR", 1},

    {"OP_GET_LOCAL", 1},
    {"OP_SET_LOCAL", 1},

    {"OP_CALL_C_FUNCTION", 0}, //SPECIAL emittedCount variable
    {"OP_CALL_SF_FUNCTION", 0}, //SPECIAL emittedCount variable

    {"OP_INC_LOCAL", 2},
    {"OP_INC_PTR", 3},

    {"OP_ADD", 0},
    {"OP_SUB", 0},
    {"OP_MUL", 0},
    {"OP_DIV", 0},


    {"OP_FOR_RANGE", 2},
    {"OP_FOR_STEP_INIT", 3 },
    {"OP_LOOP_INCREMENT_JUMP", 3},

    {"OP_POP_STACK", 0},

    {"OP_EXIT", 0},
    {"OP_INVALID", 0}
};



inline void disassembleInstruction(const uint32_t* code, size_t offset) {
    uint32_t opcode = code[offset];

    Tools::printf("%04zu: ", offset);


    if (opcode < OP_INVALID) {
        Tools::printf("%s", OpCodeInfos[opcode].name);

        switch (opcode) {
            case OP_PUSH_VALUE: {
                offset++;
                Value val;
                std::memcpy(&val, &code[offset], sizeof(Value));
                offset += 2;
                Tools::printf("  Value: %s",val.toString().c_str());
            }
            break;
            case OP_CALL_C_FUNCTION: {
                offset++;
                void* cb = nullptr;
                std::memcpy(&cb, &code[offset], sizeof(void*));
                offset += 2;

                U32 argCount = code[offset++];
                Tools::printf("  Pointer: %p, Args: %u", cb, argCount);
            }
            break;
            case OP_CALL_SF_FUNCTION: {
                //TODO TEST not implemented yet
                offset++;
                void* cb = nullptr;
                std::memcpy(&cb, &code[offset], sizeof(void*));
                offset += 2;

                U32 argCount = code[offset++];
                U32 requestType = code[offset++];
                Tools::printf("  Pointer: %p, Args: %u, requestType:%u", cb, argCount, requestType);
            }
            break;
            default:
                if (OpCodeInfos[opcode].emittedCount > 0) {
                    Tools::printf(" Args: ");
                    for (U32 i = 1; i <= OpCodeInfos[opcode].emittedCount; i++ ) {
                        Tools::printf("%u ", code[offset + i]);
                    }
                }
                break;
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
