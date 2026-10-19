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
#include "core/VariableFrame.h"

namespace DreiZehn {

//-----------------------------------------------------------------------------
#define PUSH_STACK(val) (vmStack[_STK++] = (val))
#define POP_STACK() (vmStack[--_STK])
#define PEEK_STACK(distance) (vmStack[_STK - 1 - (distance)])
#define STACK_SIZE() (_STK + 1)

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
    goto *dispatch_table[advanceU32()]; \
} while(0)
#else
#define DISPATCH() goto *dispatch_table[ADVANCE_U32()]
#endif

#define DISPATCH_OPCODE(op) goto *dispatch_table[op]



inline Value runDirectThreadedVM(BytecodeChunk& chunk, size_t numLocals = 64) {

    // constexpr void* dispatch_table[] = {
    alignas(64) constexpr  void* dispatch_table[] = {
        &&handle_OP_CONST,       // push const to stack
        &&handle_OP_PUSH_VALUE,

        &&handle_OP_JUMP,
        &&handle_OP_JUMP_IF_FALSE,

        &&handle_OP_LESS_EQUAL,
        &&handle_OP_LESS,

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
        &&handle_OP_INC_EXPR,

        &&handle_OP_ADD,
        &&handle_OP_SUB,
        &&handle_OP_MUL,
        &&handle_OP_DIV,

        //Loops
        &&handle_OP_FOR_RANGE,
        &&handle_OP_LOOP_INCREMENT_JUMP,

        &&handle_OP_POP_STACK,
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
    U32 _STK = 0;
    Value vmStack[1024] = {0};

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
    handle_OP_LESS_EQUAL: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        if (a.getInt() <= b.getInt()) {
            PUSH_STACK(Value(1));
        } else {
            PUSH_STACK(NumberNullValue);
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_LESS: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        if (a.getInt() < b.getInt()) {
            PUSH_STACK(Value(1));
        } else {
            PUSH_STACK(NumberNullValue);
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_LOAD_VAR:{
        // FIXME cache !
        PUSH_STACK( Globals::MasterVariableFrame->getVariable(ADVANCE_U32()) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_VAR:{
        U32 varID = ADVANCE_U32();
        Globals::MasterVariableFrame->setVariable(varID, POP_STACK());
        DISPATCH();
    }
    // -------------------------------
    handle_OP_GET_LOCAL: {
        U32 slot = ADVANCE_U32();
        assert(slot < locals.size() && "RUNTIME ERROR: LOCAL REGISTER OUT OF BOUNDS");
        PUSH_STACK(locals[slot]);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SET_LOCAL: {
        U32 slot = ADVANCE_U32();
        if (slot >= locals.size()) {
            locals.resize(slot + 1);
        }
        //FIXME Garbage Colletion
        locals[slot] = POP_STACK();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_C_FUNCTION: {
        void* ptr = ADVANCE_PTR();
        FunctionMap::CallBack* cb = static_cast<FunctionMap::CallBack*>(ptr);

        U32 argCount = ADVANCE_U32();
        ValueType valType = static_cast<ValueType>(ADVANCE_U32());

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

            if (valType != ValueType::NONE)
                PUSH_STACK(returnValue);
        }

        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_SF_FUNCTION: {
        //FIXME
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
    handle_OP_INC_EXPR: {
        {
            // FIXME heavy metal! may crash no checks
            Expression* expr = static_cast<Expression*>(ADVANCE_PTR());
            Value* value = expr->evaluatePtr(*Globals::gCurEnv);
            S32 inc = ADVANCE_S32();
            // FIXME double ?!
            *value = Value(value->getInt() + inc);
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_ADD: {
        Value b = POP_STACK();
        Value a = POP_STACK();
        if (a.isInt() && b.isInt())
            PUSH_STACK(Value(a.asFastInt() + b.asFastInt()));
        else if (a.isString() || b.isString())
            PUSH_STACK(Value(a.getStringRef() + b.getStringRef()));
        else
            PUSH_STACK(Value(a.getDouble() + b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SUB: {
        Value b = POP_STACK();
        Value a = POP_STACK();
        if (a.isInt() && b.isInt())
            PUSH_STACK(Value(a.asFastInt() - b.asFastInt()));
        else
            PUSH_STACK(Value(a.getDouble() - b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_MUL: {
        Value b = POP_STACK();
        Value a = POP_STACK();
        if (a.isInt() && b.isInt())
            PUSH_STACK(Value(a.asFastInt() * b.asFastInt()));
        else
            PUSH_STACK(Value(a.getDouble() * b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_DIV: {
        Value b = POP_STACK();
        Value a = POP_STACK();
        if (b.getDouble() == 0.0) {
            Tools::errorf("Runtime Error: Division by 0!");
            DISPATCH_OPCODE(OP_INVALID);
        }
        if (a.isInt() && b.isInt())
            PUSH_STACK(Value(a.asFastInt() / b.asFastInt()));
        else
            PUSH_STACK(Value(a.getDouble() / b.getDouble()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_RANGE: {
        U32 exitIP = ADVANCE_U32();
        // // get count
        Value countValue = POP_STACK();
        U32 count = countValue.getUInt();
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
    handle_OP_LOOP_INCREMENT_JUMP: {
        U32 iterSlot = ADVANCE_U32(); // variable
        U32 targetIP = ADVANCE_U32(); // body start
        S32 increment = ADVANCE_S32(); // increment

        U32 current = PEEK_STACK(0).asUInt();
        U32 limit   = PEEK_STACK(1).asUInt();

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
//     FIXME BREAK
//     U32 exitIP = advanceU32();   //
//     U32 popCount = advanceU32(); // ForRange uses 2 for example!
//
//     // 1. Stack sauber aufräumen
//     for (U32 i = 0; i < popCount; i++) {
//         popStack();
//     }
//
//     // 2. Aus der Schleife springen
//     ip = exitIP;
//     DISPATCH();
    // -------------------------------
    handle_OP_POP_STACK: {
        POP_STACK();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_EXIT: {
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("STACK COUNT: %zu, RETURN VALUE IS: %s\n", stackSize(),(stackSize()==0 ? NumberNullValue : peekStack(0)).toString().c_str());
        #endif
        //FIXME Garbage Colletion on locals!

        return _STK == 0 ? NumberNullValue : PEEK_STACK(0);
    }
    // -------------------------------
    handle_OP_INVALID: {
        // assert(false && "RUNTIME ERROR IN VM INVALID BYTE CODE!");
        return Value();
    }

    // #undef DISPATCH
    // #undef DISPATCH_OPCODE

    Tools::errorf("SHOULD NOT BE HERE !!! Did you forget OP_EXIT?");
    return NumberNullValue;
}

//-----------------------------------------------------------------------------
} // namespace DreiZehn
