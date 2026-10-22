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
#include "Environment.h"
#include "core/AstNode.h"
#include "core/VariableFrame.h"
#include "VMStructure.h"
#include "core/FunctionMap.h"
#include <core/BaseObjects.h>

namespace DreiZehn {
    //-----------------------------------------------------------------------------
    #ifndef MAX_BC_STACK_SIZE
    #define MAX_BC_STACK_SIZE  1024
    #endif


    // #define PUSH_STACK(val)      (vmStack[_STK++] = (val))
    #ifdef DREIZEHN_DEBUG
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

    #ifdef DREIZEHN_DEBUG
    #define POP_STACK() \
    ((_STK < 1) ? (Tools::PrintRuntimeError("VM Stack Underflow!"), exit(1), vmStack[0]) : vmStack[--_STK])
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

    #define PATCH_PTR( pos, ptr) \
    do { \
        std::memcpy(&code[pos], &ptr, sizeof(void*)); \
    } while(0)

    // magic macro:
    #ifdef DEBUG_TRACE_EXECUTION
    #define DISPATCH() \
    do { \
        disassembleInstruction(code, ip, _STK, callStack.size(), callStack.back().mLocals.size()); \
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
        &&handle_OP_JUMP_CLEAN_STACK,

        &&handle_OP_JUMP_IF_TRUE_NO_POP,
        &&handle_OP_JUMP_IF_FALSE_NO_POP,

        &&handle_OP_JUMP_IF_FALSE,
        &&handle_OP_JUMP_IF_TRUE,

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

        // Global variable
        &&handle_OP_LOAD_VAR,
        &&handle_OP_SAVE_VAR,

        // Local variable
        &&handle_OP_GET_LOCAL,   // fetch var
        &&handle_OP_SET_LOCAL,   // write var

        // Object Field
        &&handle_OP_LOAD_OBJECT_FIELD,
        &&handle_OP_SAVE_OBJECT_FIELD,

        // Object Methods
        &&handle_OP_CALL_OBJECT_METHOD,
        &&handle_OP_SAVE_OBJECT_METHOD,

        // Array Var
        &&handle_OP_LOAD_ARRAY,
        &&handle_OP_SAVE_ARRAY,

        // Functions
        &&handle_OP_CALL_C_FUNCTION,
        &&handle_OP_CALL_SF_FUNCTION,

        // Math
        &&handle_OP_INC_LOCAL,

        &&handle_OP_ADD_LOCAL, // += using emitted value
        &&handle_OP_SUB_LOCAL, // -= using emitted value
        &&handle_OP_MUL_LOCAL, // *= using emitted value
        &&handle_OP_DIV_LOCAL, // /= using emitted value

        &&handle_OP_ADD,
        &&handle_OP_SUB,
        &&handle_OP_MUL,
        &&handle_OP_DIV,

        &&handle_OP_NOT,

        //Loops
        &&handle_OP_FOR_RANGE,
        &&handle_OP_FOR_STEP_INIT,
        &&handle_OP_LOOP_INCREMENT_JUMP,

        &&handle_OP_FOR_EACH_INIT,
        &&handle_OP_FOR_EACH_INCREMENT_JUMP,

        &&handle_OP_CREATE_KEYVALUE,

        &&handle_OP_POP_STACK,

        &&handle_OP_RETURN,

        &&handle_OP_EXIT,        // bye bye
        &&handle_OP_INVALID
    };

    if (chunk.mByteCodes.empty()) {
        return Value(0.0);
    }
    // -----------------------
    // stack register
    S32 _STK = 0;
    // Value vmStack[MAX_BC_STACK_SIZE] = {0};

    // TEST
    std::vector<Value>vmStack;
    vmStack.resize(MAX_BC_STACK_SIZE);


    uint32_t  ip = 0;
    uint32_t* code = chunk.mByteCodes.data();
    size_t codeSize = chunk.mByteCodes.size();

    // -----------------------
    std::vector<CallStack> callStack;
    callStack.push_back({ip});
    callStack.back().mLocals.resize(numLocals);
    std::vector<Value>* localsPtr = &callStack.back().mLocals;
    // -----------------------

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

    handle_OP_JUMP_CLEAN_STACK:{
        U32 count = ADVANCE_U32();
        ip = PEEK_U32();
        for (U32 i = 0; i < count; i++ ) {
            (void) POP_STACK();
        }
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

        if (!condition.getBool()) {
            ip = PEEK_U32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_JUMP_IF_TRUE: {
        Value condition = POP_STACK();
        if (condition.getBool()) {
            ip = PEEK_U32();
        } else {
            ip++; //contiune
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_LESS: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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

    // -------------------------------------------------------------------------
    handle_OP_GREATER_EQUAL: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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

        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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

        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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
    // // -------------------------------
    handle_OP_BIT_AND: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l & r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_BIT_OR: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l | r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_BIT_XOR: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l ^ r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SHL: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l << r) );
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SHR: {
        Value rVal = POP_STACK();
        Value lVal = POP_STACK();
        U32 l = lVal.getUInt();
        U32 r = rVal.getUInt();
        PUSH_STACK ( Value( l >> r) );
        DISPATCH();
    }

    // -------------------------------
    handle_OP_MODULO: {

        Value rVal = POP_STACK();
        Value lVal = POP_STACK();

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
    // Global variable
    // -------------------------------------------------------------------------
    handle_OP_LOAD_VAR:{
        U32 symID = ADVANCE_U32();
        U32 ptrPOS = ip;
        void* rawPtr = ADVANCE_PTR();
        Value* valuePtr = nullptr;

        if (rawPtr) {
            valuePtr = static_cast<Value*>(rawPtr);
            PUSH_STACK(*valuePtr);
            DISPATCH();
        }
        VariableFrame* frame = Globals::MasterVariableFrame;
        valuePtr = frame->getVariablePtr(symID);
        PATCH_PTR(ptrPOS,valuePtr);
        PUSH_STACK(*valuePtr);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_VAR:{
        U32 symID = ADVANCE_U32();
        U32 ptrPOS = ip;
        void* rawPtr = ADVANCE_PTR();
        Value* valuePtr = nullptr;

        if (rawPtr) {
            valuePtr = static_cast<Value*>(rawPtr);
            *valuePtr = PEEK_STACK(0);
            DISPATCH();
        }
        VariableFrame* frame = Globals::MasterVariableFrame;
        valuePtr = frame->getVariablePtr(symID, true);
        PATCH_PTR(ptrPOS,valuePtr);
        *valuePtr = PEEK_STACK(0);


        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Object Field
    // -------------------------------------------------------------------------
    handle_OP_LOAD_OBJECT_FIELD: {

        U32 fieldSymId = ADVANCE_U32();
        U32 saveObjPtrPos = ip;
        void* rawObjectPtr = ADVANCE_PTR();
        U32 saveFieldPtrPos = ip;
        void* rawFieldValuePtr = ADVANCE_PTR();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            Tools::errorf("RunTime Error: Object %s not found.\n",objectValue.toString().c_str());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        Value* fieldValuePtr = nullptr;
        if (rawFieldValuePtr && rawObjectPtr &&
            rawObjectPtr == (void*) obj
        ) {
            fieldValuePtr = static_cast<Value*>(rawFieldValuePtr);
        } else {
            fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
            if (!fieldValuePtr) [[unlikely]] {
                Tools::warnf("RunTime: Object %s have no field named: %s\n",
                             objectValue.toString().c_str(),
                             SymbolTable::getName(fieldSymId).c_str());
                PUSH_STACK(NumberNullValue);
                DISPATCH();
            }
            PATCH_PTR(saveObjPtrPos, obj);
            PATCH_PTR(saveFieldPtrPos, fieldValuePtr);
        }
        PUSH_STACK(*fieldValuePtr);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_OBJECT_FIELD: {

        U32 fieldSymId = ADVANCE_U32();
        U32 saveObjPtrPos = ip;
        void* rawObjectPtr = ADVANCE_PTR();
        U32 saveFieldPtrPos = ip;
        void* rawFieldValuePtr = ADVANCE_PTR();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            Tools::errorf("RunTime Error: Object %s not found.\n",objectValue.toString().c_str());
            DISPATCH();
        }

        ValueObject* obj = objectValue.asPointerObject();
        Value* fieldValuePtr = nullptr;
        if (rawFieldValuePtr && rawObjectPtr &&
            rawObjectPtr == (void*) obj
        ) {
            fieldValuePtr = static_cast<Value*>(rawFieldValuePtr);
        } else {
            fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
            if (!fieldValuePtr) [[unlikely]] {
                Tools::warnf("RunTime: Object %s have no field named: %s\n",
                             objectValue.toString().c_str(),
                             SymbolTable::getName(fieldSymId).c_str());
                DISPATCH();
            }
            PATCH_PTR(saveObjPtrPos, obj);
            PATCH_PTR(saveFieldPtrPos, fieldValuePtr);
        }
        *fieldValuePtr = PEEK_STACK(0);

        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Object methods
    // -------------------------------------------------------------------------
    handle_OP_CALL_OBJECT_METHOD : {
        // pull method name id
        U32 symID = ADVANCE_U32();
        U32 argCount = ADVANCE_U32();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            Tools::errorf("RunTime Error: Object %s not found.\n",objectValue.toString().c_str());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        if (!obj) {
            Tools::errorf("RunTime Error: Invalid Object: %s.\n", objectValue.toString().c_str());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        {
            std::vector<Value> evaluatedArgs(argCount);
            for (int i = (int)argCount - 1; i >= 0; i--) {
                evaluatedArgs[i] = POP_STACK();
            }

            Value returnValue = NumberNullValue;
            bool success = obj->onMethodCall(symID, evaluatedArgs, returnValue);
            if (!success) {
                Tools::errorf("Runtime Error in native function call\n");
            }
            PUSH_STACK(returnValue);
        }

        DISPATCH();
    }
    // -----------------
    handle_OP_SAVE_OBJECT_METHOD : {
        // pull method name id
        U32 symID = ADVANCE_U32();
        U32 argCount = ADVANCE_U32();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            Tools::errorf("RunTime Error: Object %s not found.\n",objectValue.toString().c_str());
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        if (!obj) {
            Tools::errorf("RunTime Error: Invalid Object: %s.\n", objectValue.toString().c_str());
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }

        Value* destPrt = nullptr;
        {
            std::vector<Value> evaluatedArgs(argCount);
            for (int i = (int)argCount - 1; i >= 0; i--) {
                evaluatedArgs[i] = POP_STACK();
            }

            destPrt =  obj->onMethodCallGetAssignPtr(symID, evaluatedArgs);
        }
        if (!destPrt) [[unlikely]] {
            Tools::errorf("Runtime Error in method call. cant find method\n");
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }

        *destPrt = PEEK_STACK(0);
        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Array variable
    // -------------------------------------------------------------------------
    handle_OP_LOAD_ARRAY: {
        Value indexValue = POP_STACK();
        Value varValue = POP_STACK();

        if (!varValue.isPointer()) {
            Tools::PrintRuntimeError("Array is not an Object!");
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        const Value* resultPtr = nullptr;
        if (indexValue.isNumber()) [[likely]] {
             resultPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.asUInt());
        } else if (indexValue.isString()) {
            resultPtr =varValue.asPointerObject()->onGetArrayByStringPtr(indexValue.getStringRef());
        }
        PUSH_STACK(resultPtr ? *resultPtr : NumberNullValue);
        DISPATCH();
    }
    // ---------------------
    handle_OP_SAVE_ARRAY: {
        Value indexValue = POP_STACK();
        Value varValue = POP_STACK();
        Value assignValue = PEEK_STACK(0);

        if (!varValue.isPointer()) {
            Tools::PrintRuntimeError("Array is not an Object!");
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }
        Value* resultPtr = nullptr;
        if (indexValue.isNumber()) [[likely]] {
            resultPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.asUInt());
        } else if (indexValue.isString()) {
            resultPtr =varValue.asPointerObject()->onGetArrayByStringPtr(indexValue.getStringRef());
        }
        if (resultPtr) [[likely]]{
            *resultPtr = assignValue;
            DISPATCH();
        }
        PEEK_STACK(0) = NumberNullValue;
        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Local variable
    // -------------------------------------------------------------------------

    handle_OP_GET_LOCAL: {
        // frame pointer for call stack
        U32 slot = ADVANCE_U32();
        PUSH_STACK((*localsPtr)[slot]);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SET_LOCAL: {
        // frame pointer for call stack
        U32 slot = ADVANCE_U32();
        Value* val = nullptr;
        if (slot >= localsPtr->size()) {
            localsPtr->resize(slot + 1);
            val = &(*localsPtr)[slot];
        } else {
            val = &(*localsPtr)[slot];
            if (val->isPointer()) val->asPointerObject()->setAssigned(false);
        }
        *val = PEEK_STACK(0);
        if (val->isPointer()) val->asPointerObject()->setAssigned(true);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_C_FUNCTION: {
        // pull function name id
        U32 symID = ADVANCE_U32();
        U32 ptrPOS = ip;
        void* rawPtr = ADVANCE_PTR();
        FunctionMap::CallBack* callPtr = nullptr;
        if (!rawPtr) {
            callPtr = FunctionMap::GetCFunction(symID);
            PATCH_PTR(ptrPOS, callPtr);
        } else {
            callPtr = static_cast<FunctionMap::CallBack*>(rawPtr);
        }
        U32 argCount = ADVANCE_U32();

        {
            std::vector<Value> evaluatedArgs(argCount);
            for (int i = (int)argCount - 1; i >= 0; i--) {
                evaluatedArgs[i] = POP_STACK();
            }

            Value returnValue = NumberNullValue;
            bool success = (*callPtr)(evaluatedArgs, returnValue);

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
        U32 varCount = ADVANCE_U32();

        callStack.back().mIP = ip;
        callStack.push_back({targetIP});
        localsPtr = &callStack.back().mLocals;
        localsPtr->resize(argCount + varCount);

        for (int i = (int)argCount - 1; i >= 0; i--) {
            Value param = POP_STACK();
            U32 slot =  i;
            (*localsPtr)[slot] = param;
            // Tools::printf("** PARAM IS: %s _STK=%u  _CS=%zu framePointer is:%u argCount:%d locals:%zu\n",
            //               param.toString().c_str(), callStack.size(), _STK,framePointer, argCount, locals.size());
        }


        ip = targetIP;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_INC_LOCAL: {
        U32 slot = ADVANCE_U32();
        S32 inc = ADVANCE_S32();
        Value& val = (*localsPtr)[slot];
        if (val.isInt()) [[likely]] {
            val = Value(val.asFastInt() + inc);
            PUSH_STACK(val);
            DISPATCH();
        }

        val = Value(val.getInt() + inc);
        PUSH_STACK(val);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_ADD_LOCAL: {
        U32 slot = ADVANCE_U32();
        Value b = POP_STACK();
        Value& val = (*localsPtr)[slot];
        if (val.isDouble() && b.isDouble()) [[likely]] {
            val = Value(val.asFastDouble() + b.asFastDouble());
            PUSH_STACK(val);
            DISPATCH();
        }
        val = Value(val.getDouble() + b.getDouble());
        PUSH_STACK(val);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SUB_LOCAL: {
        U32 slot = ADVANCE_U32();
        Value b = POP_STACK();
        Value& val = (*localsPtr)[slot];
        if (val.isDouble() && b.isDouble()) [[likely]] {
            val = Value(val.asFastDouble() - b.asFastDouble());
            PUSH_STACK(val);
            DISPATCH();
        }
        val = Value(val.getDouble() - b.getDouble());
        PUSH_STACK(val);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_MUL_LOCAL: {
        U32 slot = ADVANCE_U32();
        Value b = POP_STACK();
        Value& val = (*localsPtr)[slot];
        if (val.isDouble() && b.isDouble()) [[likely]] {
            val = Value(val.asFastDouble() * b.asFastDouble());
            PUSH_STACK(val);
            DISPATCH();
        }
        val = Value(val.getDouble() * b.getDouble());
        PUSH_STACK(val);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_DIV_LOCAL: {
        U32 slot = ADVANCE_U32();
        Value b = POP_STACK();
        Value& val = (*localsPtr)[slot];
        if (val.isDouble() && b.isDouble()) [[likely]] {
            val = Value(val.asFastDouble() / b.asFastDouble());
            PUSH_STACK(val);
            DISPATCH();
        }
        val = Value(val.getDouble() / b.getDouble());
        PUSH_STACK(val);
        DISPATCH();
    }
    // -------------------------------
    // -------------------------------
    handle_OP_ADD: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        if (a.isDouble() && b.isDouble())  [[likely]]
        {
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
    // -------------------------------
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
    handle_OP_MUL: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        // if (((a.mBits & QNAN_MASK) != QNAN_MASK) && ((a.mBits & QNAN_MASK) != QNAN_MASK)) [[likely]] {
        //     PUSH_STACK(Value(std::bit_cast<double>(a.mBits) * std::bit_cast<double>(b.mBits) ));
        //     DISPATCH();
        // }

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
    handle_OP_DIV: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        const bool l_is_double = a.isDouble();
        const bool r_is_double = b.isDouble();
        if (l_is_double && r_is_double) [[likely]] {
            double r = b.asFastDouble();
            if (r == 0.0) [[unlikely]] DISPATCH_OPCODE(OP_INVALID);;
            PUSH_STACK(Value(a.asFastDouble() / r));
            DISPATCH();
        }

        const bool is_both_int = a.isInt() && b.isInt();
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
    handle_OP_NOT: {
        Value a = POP_STACK();
        PUSH_STACK(Value(!a.getBool()));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_RANGE: {
        U32 exitIP = ADVANCE_U32();
        U32 iterSlot = ADVANCE_U32();
        // // get count
        Value countValue = POP_STACK();
        S32 count = countValue.getInt();
        if (count == 0) {
            ip = exitIP;
            DISPATCH();
        }

        PUSH_STACK(Value(count - 1)); // limit
        PUSH_STACK(Value(0));     // current value


        (*localsPtr)[iterSlot] = 0;
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

        (*localsPtr)[iterSlot] = start;

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
            (*localsPtr)[iterSlot] = current;    // update iter var

            ip = targetIP;
        } else {
            (void) POP_STACK(); // pop index
            (void) POP_STACK(); // pop limit
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_EACH_INIT:{
        U32 exitIP = ADVANCE_U32();
        U32 iterSlot = ADVANCE_U32();
        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) {
            Tools::PrintRuntimeError("forEach without a object");
            ip = exitIP;
            DISPATCH();
        }
        size_t count = objectValue.asPointerObject()->onGetArraySize();
        if (count == 0) {
            ip = exitIP;
            DISPATCH();
        }

        PUSH_STACK(objectValue); // current object
        PUSH_STACK(Value((S32) count)); // count
        PUSH_STACK(Value(0));    // current iter value

        (*localsPtr)[iterSlot] =  *objectValue.asPointerObject()->onGetArrayIndexPtr(0);
        DISPATCH(); //jump to body
    }
    // -------------------------------
    handle_OP_FOR_EACH_INCREMENT_JUMP:{
        U32 iterSlot = ADVANCE_U32(); // variable
        U32 targetIP = ADVANCE_U32(); // body start

        S32 currentIter = PEEK_STACK(0).asInt();
        S32 count = PEEK_STACK(1).asInt();
        ValueObject* obj = PEEK_STACK(2).asPointerObject();
        currentIter++;
        if (currentIter < count) {
             PEEK_STACK(0) = Value(currentIter);
            (*localsPtr)[iterSlot] =  *obj->onGetArrayIndexPtr(currentIter);
             ip = targetIP;
        } else {
            (void) POP_STACK(); // pop object
            (void) POP_STACK(); // pop index
            (void) POP_STACK(); // pop limit
        }
        DISPATCH();
    }

    // -------------------------------
    handle_OP_CREATE_KEYVALUE: {
        Value valueVal = POP_STACK();
        Value keyVal = POP_STACK();

        if (!keyVal.isString())  {
            Tools::PrintRuntimeError("KeyValueExpression value must be a string,.");
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }

        auto obj = new KeyValueObject(keyVal.getStringRef(), valueVal);

        PUSH_STACK(Value(obj));

        DISPATCH();
    }
    // -------------------------------
    handle_OP_POP_STACK: {
        (void) POP_STACK();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_RETURN: {

        if (callStack.size() <  2) {
            DISPATCH_OPCODE(OP_EXIT);
        }

        Value returnValue = POP_STACK();

        callStack.pop_back();
        localsPtr = &callStack.back().mLocals;

        PUSH_STACK(returnValue);

        ip = callStack.back().mIP;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_EXIT: {
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** STACK COUNT: %zu\n", STACK_SIZE());
        #endif



        return _STK == 0 ? NumberNullValue : PEEK_STACK(0);
    }
    // -------------------------------
    handle_OP_INVALID: {
        // assert(false && "RUNTIME ERROR IN VM INVALID BYTE CODE!");
        return Value();
    }


    // Tools::errorf("SHOULD NOT BE HERE !!! Did you forget OP_EXIT?");
    return NumberNullValue;
}}
