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

// #ifdef DREIZEHN_DEBUG
    size_t disassembleInstruction(const uint32_t* code, size_t offset, const S32 stk, const size_t callstk, size_t localsSize) {
        uint32_t opcode = code[offset];

        char buffer[512] = {};
        size_t pos = 0;
        auto append = [&](const char* fmt, auto... args) {
            if (pos < sizeof(buffer)) {
                int written = snprintf(buffer + pos, sizeof(buffer) - pos, fmt, args...);
                if (written > 0) {
                    pos += static_cast<size_t>(written);
                }
            }
        };

        append("%04zu: [%02u]", offset, opcode);

        if (opcode < OP_INVALID) {
            append("%-20s", OpCodeInfos[opcode].name);

            switch (opcode) {
                case OP_PUSH_VALUE: {
                    offset++;
                    Value val;
                    std::memcpy(&val, &code[offset], sizeof(Value));
                    offset += 2;
                    append("  Value: %s", val.toString().c_str());
                    break;
                }
                case OP_CALL_C_FUNC_VOID:
                case OP_CALL_C_FUNCTION: {
                    offset++;
                    U32 symId = code[offset++];
                    void* cb = nullptr;
                    std::memcpy(&cb, &code[offset], sizeof(void*));
                    offset += 2;
                    U32 argCount = code[offset++];
                    append("%s :: SymId:%u Pointer: %p, Arg: %u", SymbolTable::getName(symId).c_str(), symId, cb, argCount);
                    break;
                }
                default: {
                    U32 count = OpCodeInfos[opcode].emittedCount;
                    append(" %s", "[ ");
                    if (count > 0) {
                        for (U32 i = 1; i <= count; i++) {
                            if (i > 1) append("%s",", ");
                            append("%u", code[offset + i]);
                        }
                    } else {
                        append("%s", "-");
                    }
                    append(" %s", "]");
                    offset += 1 + count;
                    break;
                }
            }
        } else {
            append("UNKNOWN_OPCODE [%u]", opcode);
            offset++;
        }

        if (stk >= 0) {
            append(", _STK[%d], CF:%zu (%zu)", stk, callstk, localsSize);
        }
        Tools::printf("%s\n", buffer);

        return offset;
    }

    void disassembleCode(const uint32_t* code, size_t codeSize) {
        Tools::printf("=== BYTECODE DUMP ===\n");
        size_t offset = 0;
        while (offset < codeSize) {
            offset = disassembleInstruction(code, offset);
        }
        Tools::printf("=====================\n");
    }



// void disassembleInstruction(const uint32_t* code, size_t offset, const S32 stk, const size_t callstk, size_t localsSize) {
//     uint32_t opcode = code[offset];
//
//     Tools::printf("%04zu: ", offset);
//
//
//     if (opcode < OP_INVALID) {
//         Tools::printf("%s", OpCodeInfos[opcode].name);
//
//         switch (opcode) {
//             case OP_PUSH_VALUE: {
//                 offset++;
//                 Value val;
//                 std::memcpy(&val, &code[offset], sizeof(Value));
//                 offset += 2;
//                 Tools::printf("  Value: %s",val.toString().c_str());
//             }
//             break;
//             case OP_CALL_C_FUNCTION: {
//                 offset++;
//                 U32 symId = code[offset++];
//                 void* cb = nullptr;
//                 std::memcpy(&cb, &code[offset], sizeof(void*));
//                 offset += 2;
//
//                 U32 argCount = code[offset++];
//                 Tools::printf("  SymId:%u Pointer: %p, Arg: %u", symId, cb, argCount);
//             }
//             break;
//             default:
//                 if (OpCodeInfos[opcode].emittedCount > 0) {
//                     Tools::printf(" Arg: ");
//                     for (U32 i = 1; i <= OpCodeInfos[opcode].emittedCount; i++ ) {
//                         Tools::printf("%u ", code[offset + i]);
//                     }
//                 }
//                 break;
//         }
//     } else {
//         Tools::printf("UNKNOWN_OPCODE [%u]", opcode);
//     }
//     Tools::printf(", _STK[%d], CF:%zu (%zu)\n", stk, callstk, localsSize);
// }
// #endif

} // namespace
