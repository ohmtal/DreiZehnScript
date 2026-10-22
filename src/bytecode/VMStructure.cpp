//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// VM Structure
//-----------------------------------------------------------------------------
#include <vector>
#include <cstdint>
#include <cstring> //mem ...
#include "VMStructure.h"

namespace DreiZehn {
    // Garbage Collection
    CallStack::~CallStack() {
        if (!Globals::shutDownRUNNING) {
            for (auto& loc: mLocals) {
                if (loc.isPointer()) loc.asPointerObject()->setAssigned(false);
            }
        }
    }

#ifdef DREIZEHN_DEBUG
void disassembleInstruction(const uint32_t* code, size_t offset, const S32 stk, const size_t callstk, size_t localsSize) {
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
                U32 symId = code[offset++];
                void* cb = nullptr;
                std::memcpy(&cb, &code[offset], sizeof(void*));
                offset += 2;

                U32 argCount = code[offset++];
                Tools::printf("  SymId:%u Pointer: %p, Arg: %u", symId, cb, argCount);
            }
            break;
            default:
                if (OpCodeInfos[opcode].emittedCount > 0) {
                    Tools::printf(" Arg: ");
                    for (U32 i = 1; i <= OpCodeInfos[opcode].emittedCount; i++ ) {
                        Tools::printf("%u ", code[offset + i]);
                    }
                }
                break;
        }



    } else {
        Tools::printf("UNKNOWN_OPCODE [%u]", opcode);
    }
    Tools::printf(", _STK[%d], _CS[%zu], FP:%zu\n", stk, callstk, localsSize);
}
#endif

} // namespace
