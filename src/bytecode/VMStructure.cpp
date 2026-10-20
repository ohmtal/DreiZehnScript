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
void disassembleInstruction(const uint32_t* code, size_t offset) {
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
}}
