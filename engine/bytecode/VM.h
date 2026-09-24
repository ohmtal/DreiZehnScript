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
#pragma once
#include <vector>
#include <iostream>
#include <cstring>
#include "VMStructure.h"
#include "VariableFrame.h"

namespace DreiZehn {

//-----------------------------------------------------------------------------

inline Value runDirectThreadedVM(BytecodeChunk& chunk, size_t numLocals = 64) {

    constexpr void* dispatch_table[] = {
        &&handle_OP_CONST,       // push const to stack

        &&handle_OP_JUMP,
        &&handle_OP_JUMP_IF_FALSE,

        &&handle_OP_LESS_EQUAL,

        &&handle_OP_LOAD_VAR,
        &&handle_OP_SAVE_VAR,

        // for future use ...
        &&handle_OP_GET_LOCAL,   // fetch var
        &&handle_OP_SET_LOCAL,   // write var

        // Math
        &&handle_OP_INC_LOCAL,
        &&handle_OP_ADD,
        &&handle_OP_SUB,
        &&handle_OP_MUL,
        &&handle_OP_DIV,

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
    // stack register

    std::vector<Value> vmStack;
    vmStack.reserve(32);

    auto popStack = [&vmStack]() -> Value {
        assert(!vmStack.empty() && "RUNTIME ERROR INVALID STACK ACCESS!!");
        Value val = vmStack.back();
        vmStack.pop_back();
        return val;
    };
    auto pushStack = [&vmStack](Value val) {
        vmStack.push_back(val);
    };

    uint32_t  ip = 0;
    uint32_t* code = chunk.mByteCodes.data();
    size_t codeSize = chunk.mByteCodes.size();


    auto advanceU32 = [code, &ip, codeSize]() -> U32 {
        assert(ip < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW / UNEXPECTED END OF CODE");
        return code[ip++];
    };

    auto advanceS32 = [code, &ip, codeSize]() -> S32 {
        assert(ip < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW / UNEXPECTED END OF CODE");
        return (S32)code[ip++];
    };

    auto peekU32 = [code, &ip]() -> U32 {
        return code[ip];
    };
    auto peekS32 = [code, &ip]() -> S32 {
        return (S32)code[ip];
    };

    auto advanceValue = [code, &ip, codeSize]() -> Value {
        assert(ip + 1 < codeSize && "RUNTIME ERROR: BYTECODE OVERFLOW READING INLINE VALUE");
        Value val;
        std::memcpy(&val, &code[ip], sizeof(Value));
        ip += 2;
        return val;
    };

    // magic macro:
    #ifdef DEBUG_TRACE_EXECUTION
    #define DISPATCH() \
    do { \
        disassembleInstruction(code, ip); \
        goto *dispatch_table[advanceU32()]; \
    } while(0)
    #else
    #define DISPATCH() goto *dispatch_table[advanceU32()]
    #endif

    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]
    // launch first command
    DISPATCH();

    // -------------------------------
    handle_OP_CONST: {
        pushStack(chunk.mConstants[advanceU32()]);
        DISPATCH();

    }
    // -------------------------------
    handle_OP_JUMP: {
        ip = peekU32();
        DISPATCH();
    }

    // -------------------------------
    handle_OP_JUMP_IF_FALSE: {
        Value condition = popStack();

        if (condition.getInt() == 0) {
            ip = peekU32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_LESS_EQUAL: {
        Value b = popStack();
        Value a = popStack();

        if (a.getInt() <= b.getInt()) {
            pushStack(Value(1));
        } else {
            pushStack(Value(0));
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_LOAD_VAR:{
        // lookup
        pushStack(
            gCurrentFrame->getVariable(advanceU32())
        );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_VAR:{
        U32 varID = advanceU32();
        gCurrentFrame->setVariable(varID, popStack());
        DISPATCH();
    }
    // -------------------------------
    handle_OP_GET_LOCAL: {
        U32 slot = advanceU32();
        assert(slot < locals.size() && "RUNTIME ERROR: LOCAL REGISTER OUT OF BOUNDS");
        pushStack(locals[slot]);
        DISPATCH();
    }

    // -------------------------------
    handle_OP_SET_LOCAL: {
        U32 slot = advanceU32();
        if (slot >= locals.size()) {
            locals.resize(slot + 1);
        }
        locals[slot] = popStack();
        DISPATCH();
    }

    // -------------------------------
    handle_OP_INC_LOCAL: {
        U32 slot = advanceU32();
        S32 inc = advanceS32();
        locals[slot] = Value(locals[slot].getInt() + inc);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_ADD: {
        Value b = popStack();
        Value a = popStack();
        pushStack(Value(a.getDouble() + b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SUB: {
        Value b = popStack();
        Value a = popStack();
        pushStack(Value(a.getDouble() - b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_MUL: {
        Value b = popStack();
        Value a = popStack();
        pushStack(Value(a.getDouble() * b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_DIV: {
        Value b = popStack();
        Value a = popStack();
        if (b.getDouble() == 0.0) {
            Tools::errorf("Runtime Error: Division by 0!");
            DISPATCH_OPCODE(OP_INVALID);
        }
        pushStack(Value(a.getDouble() / b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_EXIT: {
        return vmStack.empty() ? Value(0) : vmStack.back();
    }
    // -------------------------------
    handle_OP_INVALID: {
        // assert(false && "RUNTIME ERROR IN VM INVALID BYTE CODE!");
        return Value();
    }

    #undef DISPATCH
    #undef DISPATCH_OPCODE

    Tools::errorf("SHOULD NOT BE HERE !!! Did you forget OP_EXIT?");
    return Value(0);
}

//-----------------------------------------------------------------------------
} // namespace DreiZehn
