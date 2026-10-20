//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// The power machine :)
//  1. Build AST
//  2. chunk.rawInstructions
//  3. threadChunk(chunk)
//  4. runDirectThreadedVM(chunk)

//-----------------------------------------------------------------------------
#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>
#include "core/AstNode.h"
#include "core/VariableFrame.h"
#include "VMStructure.h"

namespace DreiZehn {
    //-----------------------------------------------------------------------------
    #ifndef MAX_BC_STACK_SIZE
    #define MAX_BC_STACK_SIZE 1024
    #endif

    #ifndef MAX_BC_CALL_STACK
    #define MAX_BC_CALL_STACK 1024
    #endif


    // #define PUSH_STACK(val)      (vmStack[_STK++] = (val))
    #ifdef _DEBUG
    #define PUSH_STACK(val) \
    do { \
        if (_STK >= MAX_BC_STACK_SIZE) { \
            Tools::PrintRuntimeError("VM Stack Overflow!"); \
            exit(1); \
        } \
        vmStack[_STK++] = (val); \
    } while(0)
    #else
    #define PUSH_STACK(val) (vmStack[_STK++] = (val))
    #endif

    #ifdef _DEBUG
    inline Value POP_STACK() {
        if (_STK < 1) {
            Tools::PrintRuntimeError("VM Stack Underflow!");
            exit(1);
        }
        return vmStack[--_STK];
    }
    #else
    #define POP_STACK() (vmStack[--_STK])
    #endif

    // #define POP_STACK()          (vmStack[--_STK])
    #define PEEK_STACK(distance) (vmStack[_STK - 1 - (distance)])
    #define STACK_SIZE()         (_STK)


    #define ADVANCE_U32() code[ip++]
    #define ADVANCE_S32() (S32)code[ip++]
    #define PEEK_U32() code[ip]
    #define PEEK_S32() (S32)code[ip]
    #define ADVANCE_VALUE()  (*(Value*)&code[(ip += 2) - 2])
    #define ADVANCE_PTR() (*(void**)&code[(ip += 2) - 2])

    // magic macro:
    #ifdef DEBUG_TRACE_EXECUTION
    #define DISPATCH() \
    do { \
        disassembleInstruction(code, ip); \
        goto *dispatch_table[ADVANCE_U32()]; \
    } while(0)
    #else
    #define DISPATCH() goto *dispatch_table[ADVANCE_U32()]
    #endif

    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
Value runDirectThreadedVM(BytecodeChunk& chunk, size_t numLocals) {

    // constexpr void* dispatch_table[] = {
    alignas(64) constexpr  void* dispatch_table[] = {
        &&handle_OP_CONST,       // push const to stack
        &&handle_OP_PUSH_VALUE,

        &&handle_OP_JUMP,

        &&handle_OP_JUMP_IF_TRUE_NO_POP,
        &&handle_OP_JUMP_IF_FALSE_NO_POP,
        &&handle_OP_JUMP_IF_FALSE,

        &&handle_OP_LESS_EQUAL,
        &&handle_OP_LESS,

        &&handle_OP_GREATER,
        &&handle_OP_GREATER_EQUAL,
        &&handle_OP_EQUAL,
        &&handle_OP_NOT_EQUAL,

        &&handle_OP_BIT_AND,
        &&handle_OP_BIT_OR,
        &&handle_OP_BIT_XOR,
        &&handle_OP_SHL,
        &&handle_OP_SHR,
        &&handle_OP_MODULO,

        &&handle_OP_LOAD_VAR,
        &&handle_OP_SAVE_VAR,

        // for future use ...
        &&handle_OP_GET_LOCAL,   // fetch var
        &&handle_OP_SET_LOCAL,   // write var

        // Functions
        &&handle_OP_CALL_C_FUNCTION,
        &&handle_OP_CALL_SF_FUNCTION,

        // Math
        &&handle_OP_INC_LOCAL,
        &&handle_OP_INC_PTR,

        &&handle_OP_ADD,
        &&handle_OP_SUB,
        &&handle_OP_MUL,
        &&handle_OP_DIV,


        //Loops
        &&handle_OP_FOR_RANGE,
        &&handle_OP_FOR_STEP_INIT,
        &&handle_OP_LOOP_INCREMENT_JUMP,

        &&handle_OP_POP_STACK,

        &&handle_OP_RETURN,

        &&handle_OP_EXIT,        // bye bye
        &&handle_OP_INVALID
    };

    if (chunk.mByteCodes.empty()) {
        return Value(0.0);
    }
    // -----------------------
    // Local register
    std::vector<Value> locals;
    locals.resize(numLocals);
    // -----------------------
    U32 callStack[MAX_BC_CALL_STACK];
    U32 _CS = 0; // Call-Stack Pointer
    U32 framePointer = 0; // current local variables start point
    // -----------------------
    // stack register
    S32 _STK = 0;
    Value vmStack[MAX_BC_STACK_SIZE] = {0};

    // auto popStack = [&vmStack, &_STK]() -> Value& {
    //     assert(_STK > 0 && "RUNTIME ERROR INVALID STACK ACCESS!!");
    //     Value& val = vmStack[_STK];
    //     _STK--;
    //     return val;
    // };
    // auto pushStack = [&vmStack, &_STK](Value val) {
    //     _STK++;
    //     vmStack[_STK] = val;
    // };
    //
    // auto stackSize = [&_STK]() -> U32 {
    //     return _STK + 1;
    // };
    //
    // auto peekStack = [&vmStack, &_STK](U32 distance = 0) -> Value& {
    //     assert(distance <= _STK && "RUNTIME ERROR INVALID STACK ADDRESS");
    //     return vmStack[_STK - distance];
    // };




    // std::vector<Value> vmStack;
    // vmStack.reserve(32);
    //
    // auto popStack = [&vmStack]() -> Value {
    //     assert(!vmStack.empty() && "RUNTIME ERROR INVALID STACK ACCESS!!");
    //     Value val = vmStack.back();
    //     vmStack.pop_back();
    //     return val;
    // };
    // auto pushStack = [&vmStack](Value val) {
    //     vmStack.push_back(val);
    // };
    //
    // auto peekStack = [&vmStack](size_t distance) {
    //     assert(distance < vmStack.size() && "RUNTIME ERROR INVALID STACK ADDRESS");
    //     return vmStack[vmStack.size() - 1 - distance];
    // };

    uint32_t  ip = 0;
    uint32_t* code = chunk.mByteCodes.data();
    size_t codeSize = chunk.mByteCodes.size();


    // auto advanceU32 = [code, &ip, codeSize]() -> U32 {
    //     assert(ip < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW / UNEXPECTED END OF CODE");
    //     return code[ip++];
    // };
    // #define ADVANCE_U32() code[ip++]

    // auto advanceS32 = [code, &ip, codeSize]() -> S32 {
    //     assert(ip < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW / UNEXPECTED END OF CODE");
    //     return (S32)code[ip++];
    // };
    // #define ADVANCE_S32() (S32)code[ip++]

    // auto peekU32 = [code, &ip]() -> U32 {
    //     return code[ip];
    // };
    // #define PEEK_U32() code[ip]

    // auto peekS32 = [code, &ip]() -> S32 {
    //     return (S32)code[ip];
    // };
    // #define PEEK_S32() (S32)code[ip]

    // auto advanceValue = [code, &ip, codeSize]() -> Value {
    //     assert(ip + 1 < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW READING INLINE VALUE");
    //     Value val;
    //     std::memcpy(&val, &code[ip], sizeof(Value));
    //     ip += 2;
    //     return val;
    // };

    // #define ADVANCE_VALUE()  (*(Value*)&code[(ip += 2) - 2])

    // auto advancePointer = [code, &ip, codeSize]() -> void* {
    //     assert(ip + 1 < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW READING INLINE POINTER");
    //     void* ptr;
    //     std::memcpy(&ptr, &code[ip], sizeof(void*));
    //     ip += 2;
    //     return ptr;
    // };

    // #define ADVANCE_PTR() (*(void**)&code[(ip += 2) - 2])


    // // magic macro:
    // #ifdef DEBUG_TRACE_EXECUTION
    // #define DISPATCH() \
    // do { \
    //     disassembleInstruction(code, ip); \
    //     goto *dispatch_table[advanceU32()]; \
    // } while(0)
    // #else
    // #define DISPATCH() goto *dispatch_table[ADVANCE_U32()]
    // #endif
    //
    // #define DISPATCH_OPCODE(op) goto *dispatch_table[op]
    // launch first command

    DISPATCH();

    // -------------------------------
    handle_OP_CONST: {
        PUSH_STACK(chunk.mConstants[ADVANCE_U32()]);
        DISPATCH();

    }
    // -------------------------------
    handle_OP_PUSH_VALUE: {
        PUSH_STACK(ADVANCE_VALUE());
        DISPATCH();
    }
    // -------------------------------
    handle_OP_JUMP: {
        ip = PEEK_U32();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_JUMP_IF_TRUE_NO_POP: {
        Value condition = PEEK_STACK(0);

        if (condition.getBool() ) {
            ip = PEEK_U32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_JUMP_IF_FALSE_NO_POP: {
        Value condition = PEEK_STACK(0);

        if (!condition.getBool()) {
            ip = PEEK_U32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_JUMP_IF_FALSE: {
        Value condition = POP_STACK();

        if (condition.getInt() == 0) {
            ip = PEEK_U32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }

    // -------------------------------
    // handle_OP_LESS: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //     double l = lVal.getDouble();
    //     double r = rVal.getDouble();
    //     PUSH_STACK (Value((r - l) > EPSILON ? 1 : 0));
    //     DISPATCH();
    // }
    // // -------------------------------
    // handle_OP_LESS_EQUAL: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //     double l = lVal.getDouble();
    //     double r = rVal.getDouble();
    //     PUSH_STACK ( Value(l < (r + EPSILON) ? 1 : 0));
    //     DISPATCH();
    // }
    // -------------------------------------------------------------------------
    handle_OP_LESS: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        // if (((lVal.mBits & QNAN_MASK) != QNAN_MASK) && ((rVal.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
        //     PUSH_STACK(Value(std::bit_cast<double>(lVal.mBits) < std::bit_cast<double>(rVal.mBits) ? 1 : 0));
        //     DISPATCH();
        // }
        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
            PUSH_STACK(Value(lVal.asFastDouble() < rVal.asFastDouble() ? 1 : 0));
            DISPATCH();
        }


        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() < rVal.asFastInt() ? 1 : 0));
        } else {
            PUSH_STACK(Value(lVal.getDouble() < rVal.getDouble() ? 1 : 0));
        }
        DISPATCH();
    }

    // -------------------------------------------------------------------------
    handle_OP_LESS_EQUAL: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        // if (((lVal.mBits & QNAN_MASK) != QNAN_MASK) && ((rVal.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
        //     PUSH_STACK(Value(std::bit_cast<double>(lVal.mBits) <= std::bit_cast<double>(rVal.mBits) ? 1 : 0));
        //     DISPATCH();
        // }
        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
             PUSH_STACK(Value(lVal.asFastDouble() <= rVal.asFastDouble() ? 1 : 0));
              DISPATCH();
        }

        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() <= rVal.asFastInt() ? 1 : 0));
        } else {
            PUSH_STACK(Value(lVal.getDouble() <= rVal.getDouble() ? 1 : 0));
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_GREATER: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        // if (((lVal.mBits & QNAN_MASK) != QNAN_MASK) && ((rVal.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
        //     PUSH_STACK(Value(std::bit_cast<double>(lVal.mBits) > std::bit_cast<double>(rVal.mBits) ? 1 : 0));
        //     DISPATCH();
        // }
        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
            PUSH_STACK(Value(lVal.asFastDouble() > rVal.asFastDouble() ? 1 : 0));
            DISPATCH();
        }


        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() > rVal.asFastInt() ? 1 : 0));
        } else {
            PUSH_STACK(Value(lVal.getDouble() > rVal.getDouble() ? 1 : 0));
        }

        DISPATCH();
    }

    // handle_OP_GREATER: {
    //
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //     double l = lVal.getDouble();
    //     double r = rVal.getDouble();
    //     PUSH_STACK (Value((l - r) > EPSILON ? 1 : 0));
    //     DISPATCH();
    //
    // }
    // -------------------------------
    // handle_OP_GREATER_EQUAL: {
    //
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //     double l = lVal.getDouble();
    //     double r = rVal.getDouble();
    //     PUSH_STACK ( Value(l > (r - EPSILON) ? 1 : 0));
    //     DISPATCH();
    //
    // }

    // -------------------------------------------------------------------------
    handle_OP_GREATER_EQUAL: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        // if (((lVal.mBits & QNAN_MASK) != QNAN_MASK) && ((rVal.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
        //     PUSH_STACK(Value(std::bit_cast<double>(lVal.mBits) >= std::bit_cast<double>(rVal.mBits) ? 1 : 0));
        //     DISPATCH();
        // }
        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
            PUSH_STACK(Value(lVal.asFastDouble() >= rVal.asFastDouble() ? 1 : 0));
            DISPATCH();
        }

        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() >= rVal.asFastInt() ? 1 : 0));
        } else {
            PUSH_STACK(Value(lVal.getDouble() >= rVal.getDouble() ? 1 : 0));
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_EQUAL: {

        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
            PUSH_STACK(Value(lVal.asFastDouble() == rVal.asFastDouble() ? 1 : 0));
            DISPATCH();
        }

        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() == rVal.asFastInt() ? 1 : 0));
            DISPATCH();
        }

        if (lVal.isPointer() && rVal.isPointer()) {
            PUSH_STACK (Value(lVal.asPointer() == rVal.asPointer() ? 1 : 0));
            DISPATCH();
        }

        if (lVal.isString() && rVal.isString()) {
            PUSH_STACK (lVal.getStringRef() == rVal.getStringRef());
            DISPATCH();
        }

        PUSH_STACK (Value(lVal.getDouble() == rVal.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_NOT_EQUAL: {

        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        if (lVal.isDouble() && rVal.isDouble()) [[likely]] {
            PUSH_STACK(Value(lVal.asFastDouble() != rVal.asFastDouble()));
            DISPATCH();
        }

        if (lVal.isInt() && rVal.isInt()) {
            PUSH_STACK(Value(lVal.asFastInt() != rVal.asFastInt()));
            DISPATCH();
        }

        if (lVal.isPointer() && rVal.isPointer()) {
            PUSH_STACK (Value(lVal.asPointer() != rVal.asPointer()));
            DISPATCH();
        }

        if (lVal.isString() && rVal.isString()) {
            PUSH_STACK (lVal.getStringRef() != rVal.getStringRef());
            DISPATCH();
        }

        PUSH_STACK (Value(lVal.getDouble() != rVal.getDouble()));
        DISPATCH();
    }
    // handle_OP_NOT_EQUAL: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     if (lVal.mBits == rVal.mBits) [[likely]] {
    //         PUSH_STACK(Value(0)); // Repräsentiert false
    //         DISPATCH();
    //     }
    //
    //     uint32_t l_tag = static_cast<uint32_t>((lVal.mBits & TAG_MASK) >> 48);
    //     uint32_t r_tag = static_cast<uint32_t>((rVal.mBits & TAG_MASK) >> 48);
    //
    //     const bool l_is_double = ((lVal.mBits & QNAN_MASK) != QNAN_MASK);
    //     const bool r_is_double = ((rVal.mBits & QNAN_MASK) != QNAN_MASK);
    //
    //     constexpr uint32_t ID_INT = 1;
    //     constexpr uint32_t ID_PTR = 2;
    //     constexpr uint32_t ID_STR = 4;
    //
    //     if (l_is_double && r_is_double) {
    //         PUSH_STACK(Value(std::bit_cast<double>(lVal.mBits) != std::bit_cast<double>(rVal.mBits) ? 1 : 0));
    //         DISPATCH();
    //     }
    //
    //     if (!l_is_double && !r_is_double && (l_tag == r_tag)) {
    //         switch (l_tag) {
    //             case ID_INT:
    //             case ID_PTR:
    //                 PUSH_STACK(Value(1));
    //                 break;
    //
    //             case ID_STR:
    //                 PUSH_STACK(Value(lVal.getStringRef() != rVal.getStringRef() ? 1 : 0));
    //                 break;
    //
    //             default:
    //                 PUSH_STACK(Value(1));
    //                 break;
    //         }
    //         DISPATCH();
    //     }
    //
    //     const bool l_is_int = (!l_is_double && l_tag == ID_INT);
    //     const bool r_is_int = (!r_is_double && r_tag == ID_INT);
    //
    //     if ((l_is_int && r_is_double) || (l_is_double && r_is_int)) {
    //         PUSH_STACK(Value(lVal.getDouble() != rVal.getDouble() ? 1 : 0));
    //     } else {
    //         PUSH_STACK(Value(1));
    //     }
    //
    //     DISPATCH();
    // }

    // handle_OP_NOT_EQUAL: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //     if (lVal.isPointer() && rVal.isPointer()) {
    //         PUSH_STACK (Value(lVal.asPointer() != rVal.asPointer() ? 1 : 0));
    //     } else if (lVal.isString() && rVal.isString()) {
    //         PUSH_STACK (strcmp(lVal.getStringRef().c_str(), rVal.getStringRef().c_str()) != 0);
    //     } else if (lVal.isString() || rVal.isString()) {
    //         PUSH_STACK (Value(1));
    //     } else {
    //         double l = lVal.getDouble();
    //         double r = rVal.getDouble();
    //         PUSH_STACK (Value(std::abs(l - r) < EPSILON ? 0 : 1));
    //     }
    //     DISPATCH();
    // }
    // // -------------------------------
    handle_OP_BIT_AND: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l & r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_BIT_OR: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l | r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_BIT_XOR: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l ^ r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SHL: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l << r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SHR: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l >> r) );
        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // handle_OP_BIT_AND: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     // FAST-PATH: Wenn beide getaggte Integers sind, extrahieren wir die 32-Bit direkt
    //     constexpr uint64_t INT_MARKER = QNAN_MASK | TAG_INT;
    //     if (((lVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER) && ((rVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER)) [[likely]] {
    //         uint32_t l = static_cast<uint32_t>(lVal.mBits & 0xFFFFFFFFULL);
    //         uint32_t r = static_cast<uint32_t>(rVal.mBits & 0xFFFFFFFFULL);
    //         PUSH_STACK(Value(l & r));
    //     } else {
    //         // SLOW-PATH: Fallback für Doubles oder andere Typen
    //         PUSH_STACK(Value(lVal.getUInt() & rVal.getUInt()));
    //     }
    //     DISPATCH();
    // }
    //
    // // -------------------------------------------------------------------------
    // handle_OP_BIT_OR: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     constexpr uint64_t INT_MARKER = QNAN_MASK | TAG_INT;
    //     if (((lVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER) && ((rVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER)) [[likely]] {
    //         uint32_t l = static_cast<uint32_t>(lVal.mBits & 0xFFFFFFFFULL);
    //         uint32_t r = static_cast<uint32_t>(rVal.mBits & 0xFFFFFFFFULL);
    //         PUSH_STACK(Value(l | r));
    //     } else {
    //         PUSH_STACK(Value(lVal.getUInt() | rVal.getUInt()));
    //     }
    //     DISPATCH();
    // }
    //
    // // -------------------------------------------------------------------------
    // handle_OP_BIT_XOR: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     constexpr uint64_t INT_MARKER = QNAN_MASK | TAG_INT;
    //     if (((lVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER) && ((rVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER)) [[likely]] {
    //         uint32_t l = static_cast<uint32_t>(lVal.mBits & 0xFFFFFFFFULL);
    //         uint32_t r = static_cast<uint32_t>(rVal.mBits & 0xFFFFFFFFULL);
    //         PUSH_STACK(Value(l ^ r));
    //     } else {
    //         PUSH_STACK(Value(lVal.getUInt() ^ rVal.getUInt()));
    //     }
    //     DISPATCH();
    // }
    //
    // // -------------------------------------------------------------------------
    // handle_OP_SHL: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     constexpr uint64_t INT_MARKER = QNAN_MASK | TAG_INT;
    //     if (((lVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER) && ((rVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER)) [[likely]] {
    //         uint32_t l = static_cast<uint32_t>(lVal.mBits & 0xFFFFFFFFULL);
    //         uint32_t r = static_cast<uint32_t>(rVal.mBits & 0xFFFFFFFFULL);
    //         PUSH_STACK(Value(l << r));
    //     } else {
    //         PUSH_STACK(Value(lVal.getUInt() << rVal.getUInt()));
    //     }
    //     DISPATCH();
    // }
    //
    // // -------------------------------------------------------------------------
    // handle_OP_SHR: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     constexpr uint64_t INT_MARKER = QNAN_MASK | TAG_INT;
    //     if (((lVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER) && ((rVal.mBits & QNAN_AND_TAG_MASK) == INT_MARKER)) [[likely]] {
    //         uint32_t l = static_cast<uint32_t>(lVal.mBits & 0xFFFFFFFFULL);
    //         uint32_t r = static_cast<uint32_t>(rVal.mBits & 0xFFFFFFFFULL);
    //         PUSH_STACK(Value(l >> r));
    //     } else {
    //         PUSH_STACK(Value(lVal.getUInt() >> rVal.getUInt()));
    //     }
    //     DISPATCH();
    // }

    // -------------------------------
    handle_OP_MODULO: {
        Value lVal = POP_STACK();
        Value rVal = POP_STACK();

        if (lVal.isDouble() && rVal.isDouble()) {
            double r = rVal.asFastDouble();
            if (r == 0.0) {
                Tools::PrintRuntimeError("Modulo Division by 0.0!");
                PUSH_STACK(NumberNullValue);
            } else {
                PUSH_STACK(Value(std::fmod(lVal.asFastDouble(), r)));
            }
        } else {
            int64_t l = lVal.getInt64();
            int64_t r = rVal.getInt64();
            if (r == 0) {
                Tools::PrintRuntimeError("Modulo Division by 0!");
                PUSH_STACK(NumberNullValue);
            } else {
                int64_t res = l % r;
                PUSH_STACK(Value(static_cast<uint32_t>(res)));
            }
        }
        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // handle_OP_MODULO: {
    //     Value lVal = POP_STACK();
    //     Value rVal = POP_STACK();
    //
    //     if (lVal.isInt() && rVal.isInt()) [[likely]] {
    //         int32_t r = rVal.asFastInt();
    //         if (r == 0) [[unlikely]] {
    //             Tools::PrintRuntimeError("Modulo Division by 0!");
    //             PUSH_STACK(NumberNullValue);
    //         } else {
    //             int32_t res = lVal.asFastInt() % r;
    //             PUSH_STACK(Value(res)); // Behält das Vorzeichen korrekt bei!
    //         }
    //         DISPATCH();
    //     }
    //
    //     if (((lVal.mBits & QNAN_MASK) != QNAN_MASK) && ((rVal.mBits & QNAN_MASK) != QNAN_MASK)) {
    //         double r = std::bit_cast<double>(rVal.mBits);
    //         if (r == 0.0) [[unlikely]] {
    //             Tools::PrintRuntimeError("Modulo Division by 0.0!");
    //             PUSH_STACK(NumberNullValue);
    //         } else {
    //             PUSH_STACK(Value(std::fmod(std::bit_cast<double>(lVal.mBits), r)));
    //         }
    //         DISPATCH();
    //     }
    //
    //     double r = rVal.getDouble();
    //     if (r == 0.0) [[unlikely]] {
    //         Tools::PrintRuntimeError("Modulo Division by 0.0!");
    //         PUSH_STACK(NumberNullValue);
    //     } else {
    //         PUSH_STACK(Value(std::fmod(lVal.getDouble(), r)));
    //     }
    //
    //     DISPATCH();
    // }

    // -------------------------------
    // -------------------------------
    handle_OP_LOAD_VAR:{
        PUSH_STACK( Globals::MasterVariableFrame->getVariable(ADVANCE_U32()) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_VAR:{
        U32 varID = ADVANCE_U32();
        Globals::MasterVariableFrame->setVariable(varID, POP_STACK(), true);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_GET_LOCAL: {
        // frame pointer for call stack
        U32 slot = framePointer + ADVANCE_U32();
        // // // assert(slot < locals.size() && "RUNTIME ERROR: LOCAL REGISTER OUT OF BOUNDS");
        // // // this can happen with optional parameter !!!
        // // if (slot >= locals.size()) {
        // //     PUSH_STACK(NumberNullValue);
        // // } else {
        // //     PUSH_STACK(locals[slot]);
        // // }
        PUSH_STACK(locals[slot]);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SET_LOCAL: {
        // frame pointer for call stack
        U32 slot = framePointer + ADVANCE_U32();
        if (slot >= locals.size()) {
            locals.resize(slot + 1);
        } else {
            // Garbage Colletion
            if (locals[slot].isPointer()) locals[slot].asPointerObject()->setAssigned(false);
        }
        locals[slot] = POP_STACK();
        if (locals[slot].isPointer()) locals[slot].asPointerObject()->setAssigned(true);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_C_FUNCTION: {
        void* ptr = ADVANCE_PTR();
        FunctionMap::CallBack* cb = static_cast<FunctionMap::CallBack*>(ptr);

        U32 argCount = ADVANCE_U32();

        {
            std::vector<Value> evaluatedArgs(argCount);
            for (int i = (int)argCount - 1; i >= 0; i--) {
                evaluatedArgs[i] = POP_STACK();
            }

            Value returnValue = NumberNullValue;
            bool success = (*cb)(evaluatedArgs, returnValue);

            if (!success) {
                Tools::errorf("Runtime Error in native function call\n");
            }

            PUSH_STACK(returnValue);
        }

        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_SF_FUNCTION: {
        U32 targetIP = ADVANCE_U32();
        U32 argCount = ADVANCE_U32();

        callStack[_CS++] = ip;

        PUSH_STACK(Value(static_cast<double>(framePointer)));
        PUSH_STACK(Value(static_cast<double>(argCount)));

        framePointer = _STK - 2 - argCount;

        for (S32 i = argCount - 1; i >= 0; --i) {
            locals[framePointer + i] = PEEK_STACK(2 + (argCount - 1 - i));
        }

        ip = targetIP;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_INC_LOCAL: {
        U32 slot = ADVANCE_U32();
        S32 inc = ADVANCE_S32();
        locals[slot] = Value(locals[slot].getInt() + inc);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_INC_PTR: {
        // HACK fest
        {
            Expression* expr = static_cast<Expression*>(ADVANCE_PTR());
            Value* value = expr->evaluatePtr(*Globals::gCurEnv);
            S32 inc = ADVANCE_S32();
            *value = Value(value->getInt() + inc);
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_ADD: {
        Value b = POP_STACK();
        Value a = POP_STACK();
        if (a.isDouble() && b.isDouble())  [[likely]] {
            PUSH_STACK(Value(a.asFastDouble() + b.asFastDouble()));
            DISPATCH();
        }
        if (a.isInt() && b.isInt()) {
            PUSH_STACK(Value(a.asFastInt() + b.asFastInt()));
            DISPATCH();
        }

        PUSH_STACK(Value::handleGenericAdd(a, b));
        DISPATCH();
    }

    // handle_OP_ADD: {
    //     Value b = POP_STACK();
    //     Value a = POP_STACK();
    //
    //     if (((a.mBits & QNAN_MASK) != QNAN_MASK) && ((b.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
    //         PUSH_STACK(Value(std::bit_cast<double>(a.mBits) + std::bit_cast<double>(b.mBits)));
    //         DISPATCH();
    //     }
    //
    //      if (a.isInt() && b.isInt()) {
    //         PUSH_STACK(Value(a.asFastInt() + b.asFastInt()));
    //         DISPATCH();
    //      }
    //
    //     if (a.isString() && b.isString()) {
    //         PUSH_STACK(Value(a.getStringRef() + b.getStringRef()));
    //         DISPATCH();
    //     }
    //
    //
    //
    //     uint32_t a_tag = static_cast<uint32_t>((a.mBits & TAG_MASK) >> 48);
    //     uint32_t b_tag = static_cast<uint32_t>((b.mBits & TAG_MASK) >> 48);
    //
    //     uint32_t combined_type = (a_tag << 4) | b_tag;
    //
    //     constexpr uint32_t ID_INT = 1; // binary 001
    //     constexpr uint32_t ID_STR = 4; // binary 100
    //
    //     switch (combined_type) {
    //         case (ID_INT << 4) | ID_INT:
    //             PUSH_STACK(Value(a.asFastInt() + b.asFastInt()));
    //             break;
    //
    //         case (ID_STR << 4) | ID_STR:
    //             PUSH_STACK(Value(a.getStringRef() + b.getStringRef()));
    //             break;
    //
    //         default: [[unlikely]]
    //             PUSH_STACK(Value::handleGenericAdd(a, b));
    //             break;
    //     }
    //     DISPATCH();
    // }

    // -------------------------------
    // handle_OP_SUB: {
    //     Value b = POP_STACK();
    //     Value a = POP_STACK();
    //
    //     if (((a.mBits & QNAN_MASK) != QNAN_MASK) && ((b.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
    //         PUSH_STACK(Value(std::bit_cast<double>(a.mBits) - std::bit_cast<double>(b.mBits)));
    //         DISPATCH();
    //     }
    //
    //     if (a.isInt() && b.isInt()) {
    //         PUSH_STACK(Value(a.asFastInt() - b.asFastInt()));
    //     } else {
    //         PUSH_STACK(Value(a.getDouble() - b.getDouble()));
    //     }
    //
    //     DISPATCH();
    // }

    handle_OP_SUB: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        if (a.isDouble() && b.isDouble())  [[likely]] {
            PUSH_STACK(Value(a.asFastDouble() - b.asFastDouble()));
            DISPATCH();
        }
        if (a.isInt() && b.isInt()) {
            PUSH_STACK(Value(a.asFastInt() - b.asFastInt()));
            DISPATCH();
        }
        PUSH_STACK(Value(a.getDouble() - b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    // handle_OP_MUL: {
    //     Value b = POP_STACK();
    //     Value a = POP_STACK();
    //
    //     if (((a.mBits & QNAN_MASK) != QNAN_MASK) && ((b.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
    //         PUSH_STACK(Value(std::bit_cast<double>(a.mBits) * std::bit_cast<double>(b.mBits)));
    //         DISPATCH();
    //     }
    //
    //     if (a.isInt() && b.isInt()) {
    //         int32_t result;
    //         if (!__builtin_mul_overflow(a.asFastInt(), b.asFastInt(), &result)) [[likely]] {
    //             PUSH_STACK(Value(result));
    //         } else {
    //             PUSH_STACK(Value(static_cast<double>(a.asFastInt()) * static_cast<double>(b.asFastInt())));
    //         }
    //     } else {
    //         PUSH_STACK(Value(a.getDouble() * b.getDouble()));
    //     }
    //     DISPATCH();
    // }

    handle_OP_MUL: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        if (a.isDouble() && b.isDouble())  [[likely]] {
            PUSH_STACK(Value(a.asFastDouble() * b.asFastDouble()));
            DISPATCH();
        }
        if (a.isInt() && b.isInt()) {
            int32_t result;
            if (!__builtin_mul_overflow(a.asFastInt(), b.asFastInt(), &result)) [[likely]] {
                PUSH_STACK(Value(result));
            } else {
                PUSH_STACK(Value(static_cast<double>(a.asFastInt()) * static_cast<double>(b.asFastInt())));
            }
            DISPATCH();
        }
        PUSH_STACK(Value(a.getDouble() * b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    // handle_OP_DIV: {
    //     Value b = POP_STACK();
    //     Value a = POP_STACK();
    //
    //     if (((a.mBits & QNAN_MASK) != QNAN_MASK) && ((b.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
    //         double b_val = std::bit_cast<double>(b.mBits);
    //         if (b_val == 0.0) [[unlikely]] {
    //             Tools::errorf("Runtime Error: Division by 0!");
    //             DISPATCH_OPCODE(OP_INVALID);
    //         }
    //         PUSH_STACK(Value(std::bit_cast<double>(a.mBits) / b_val));
    //         DISPATCH();
    //     }
    //
    //     if (a.isInt() && b.isInt()) {
    //         int32_t b_val = b.asFastInt();
    //         if (b_val == 0) [[unlikely]] {
    //             Tools::errorf("Runtime Error: Division by 0!");
    //             DISPATCH_OPCODE(OP_INVALID);
    //         }
    //
    //         int32_t a_val = a.asFastInt();
    //         if (a_val % b_val == 0) {
    //             PUSH_STACK(Value(a_val / b_val));
    //         } else {
    //             PUSH_STACK(Value(static_cast<double>(a_val) / static_cast<double>(b_val)));
    //         }
    //     } else {
    //         double b_val = b.getDouble();
    //         if (b_val == 0.0) [[unlikely]] {
    //             Tools::errorf("Runtime Error: Division by 0!");
    //             DISPATCH_OPCODE(OP_INVALID);
    //         }
    //         PUSH_STACK(Value(a.getDouble() / b_val));
    //     }
    //     DISPATCH();
    // }

    handle_OP_DIV: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        const bool l_is_double = a.isDouble();
        const bool r_is_double = b.isDouble();
        const bool is_both_int = a.isInt() && b.isInt();
        if (l_is_double && r_is_double) [[likely]] {
            double r = b.asFastDouble();
            if (r == 0.0) [[unlikely]] DISPATCH_OPCODE(OP_INVALID);;
            PUSH_STACK(Value(a.asFastDouble() / r));
            DISPATCH();
        }
        if (is_both_int) {
            int32_t r = b.asFastInt();
            if (r == 0) [[unlikely]] DISPATCH_OPCODE(OP_INVALID);;

            int32_t l = a.asFastInt();
            if (l % r == 0) {
                PUSH_STACK(Value(l / r));
                DISPATCH();
            }
            PUSH_STACK(Value(static_cast<double>(l) / static_cast<double>(r)));
            DISPATCH();
        }

        double r = b.getDouble();
        if (r == 0.0) [[unlikely]]  DISPATCH_OPCODE(OP_INVALID);
        PUSH_STACK(Value(a.getDouble() / r));
        DISPATCH();
    }

    // -------------------------------
    handle_OP_FOR_RANGE: {
        U32 exitIP = ADVANCE_U32();
        // // get count
        Value countValue = POP_STACK();
        S32 count = countValue.getInt();
        if (count == 0) {
            ip = exitIP;
            DISPATCH();
        }
        U32 iterSlot = ADVANCE_U32();

        PUSH_STACK(Value(count - 1)); // limit
        PUSH_STACK(Value(0));     // current value


        locals[iterSlot] = 0;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_STEP_INIT: {
        U32 exitIP = ADVANCE_U32();
        U32 incrementCodePos = ADVANCE_U32();
        U32 iterSlot = ADVANCE_U32();

        S32 step  = std::abs(POP_STACK().getInt());
        S32 end   = POP_STACK().getInt();
        S32 start = POP_STACK().getInt();

        if (step == 0 || start == end) {
            ip = exitIP;
            DISPATCH();
        }

        S32 increment = (start < end) ? step : -step;

        PUSH_STACK(Value(end));
        PUSH_STACK(Value(start));

        locals[iterSlot] = start;

        // patch increment
        code[incrementCodePos] = (U32)increment;

        DISPATCH();
    }

    // -------------------------------
    handle_OP_LOOP_INCREMENT_JUMP: {
        U32 iterSlot = ADVANCE_U32(); // variable
        U32 targetIP = ADVANCE_U32(); // body start
        S32 increment = ADVANCE_S32(); // increment

        S32 current = PEEK_STACK(0).asInt();
        S32 limit   = PEEK_STACK(1).asInt();

        current += increment;
        bool loopContinues = (increment > 0) ? (current <= limit) : (current >= limit);

        if (loopContinues) {
            PEEK_STACK(0) = Value(current); // set new value to stack
            locals[iterSlot] = current;    // update iter var

            ip = targetIP;
        } else {
            POP_STACK(); // pop index
            POP_STACK(); // pop limit
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_POP_STACK: {
        POP_STACK();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_RETURN: {

        if (_CS == 0) {
            DISPATCH_OPCODE(OP_EXIT);
        }

        Value returnValue = POP_STACK();

        U32 argCount        = static_cast<U32>(POP_STACK().getDouble());
        U32 oldFramePointer = static_cast<U32>(POP_STACK().getDouble());

        _STK = framePointer;

        framePointer = oldFramePointer;

        PUSH_STACK(returnValue);

        ip = callStack[--_CS];
        DISPATCH();
    }
    // -------------------------------
    handle_OP_EXIT: {
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("STACK COUNT: %zu\n", STACK_SIZE());
        // Tools::printf("STACK COUNT: %zu, RETURN VALUE IS: %s\n", STACK_SIZE(),(STACK_SIZE() == 0 ? NumberNullValue : PEEK_STACK(0)).toString().c_str());
        #endif
        //FIXME Garbage Colletion on locals!

        return _STK == 0 ? NumberNullValue : PEEK_STACK(0);
    }
    // -------------------------------
    handle_OP_INVALID: {
        // assert(false && "RUNTIME ERROR IN VM INVALID BYTE CODE!");
        return Value();
    }


    Tools::errorf("SHOULD NOT BE HERE !!! Did you forget OP_EXIT?");
    return NumberNullValue;
}}
