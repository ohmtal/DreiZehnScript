//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// The power machine :)
//  1. Build AST
//  2. chunk.rawInstructions
//  3. threadChunk(chunk)
//  4. runDirectThreadedVM(chunk)


// NOTE: tesing most used OP Codes
// >>>>
// #define DREIZEHN_VM_STATS
// <<<<

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
#include "VM.h"

namespace DreiZehn::VM {
    //-----------------------------------------------------------------------------
    void PrintVMError(const U32 opcode,  const std::string& p1,
                       const std::string& p2 = "", const std::string& p3 = ""){

        Tools::errorf("VM Runtime Error in %s \n\tMessage: %s %s %s\n"
        , OpCodeInfos[opcode].name
        , p1.c_str()
        , p2.c_str()
        , p3.c_str()
        );

    }
    //-----------------------------------------------------------------------------



    #ifndef MAX_BC_STACK_SIZE
    #define MAX_BC_STACK_SIZE  4096
    #endif

    // dynamic
    #ifndef INITIAL_BC_LOCALS_SIZE
    #define INITIAL_BC_LOCALS_SIZE  1024
    #endif


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
    ((_STK < 1) ? (Tools::PrintRuntimeError("VM Stack Underflow!\n"), exit(1), vmStack[0]) : vmStack[--_STK])
    #else
    #define POP_STACK() (vmStack[--_STK])
    #endif

    // #define POP_STACK()          (vmStack[--_STK])
    #define PEEK_STACK(distance) (vmStack[_STK - 1 - (distance)])
    #define STACK_SIZE()         (_STK)

    #define ADVANCE_U32() code[ip++]
    #define ADVANCE_S32() static_cast<S32>(code[ip++])
    #define ADVANCE_F32() static_cast<F32>(code[ip++])
    #define PEEK_U32() code[ip]
    #define PEEK_S32() static_cast<S32>(code[ip])
    #define PEEK_F32() static_cast<F32>(code[ip])


    // inline Value advance_value(const U32* code, U32& ip) {
    //     Value val;
    //     std::memcpy(&val, &code[ip], sizeof(Value));
    //     ip += 2;
    //     return val;
    // }
    //
    //
    // inline void* advance_ptr(const U32* code, U32& ip) {
    //     void* ptr;
    //     std::memcpy(&ptr, &code[ip], sizeof(void*));
    //     ip += 2;
    //     return ptr;
    // }
    //
    // inline void patch_ptr(U32* code, U32 pos, void* ptr) {
    //     code[pos] = 0;
    //     code[pos + 1] = 0;
    //     std::memcpy(&code[pos], &ptr, sizeof(void*));
    // }
    #define SKIP_PTR() (ip += 2)


    // magic macro:
    #ifdef DREIZEHN_VM_STATS
    #define DISPATCH() \
    do { \
        U32 op = ADVANCE_U32(); \
        op_counter[op]++;  \
        goto *dispatch_table[op]; \
    } while(0)

    #define DISPATCH_OPCODE(op) \
    do { \
        op_counter[op]++;  \
        goto *dispatch_table[op]; \
    } while(0)
    #else

    // magic macro:
    #ifdef DEBUG_TRACE_EXECUTION
    #define DISPATCH() \
    do { \
        disassembleInstruction(code, ip, _STK, callFrame.size(), callFrame.back().mLocalsCount); \
        goto *dispatch_table[ADVANCE_U32()]; \
    } while(0)

    #define DISPATCH_OPCODE(op) \
    do { \
        disassembleInstruction(code, ip, _STK, callFrame.size(), callFrame.back().mLocalsCount); \
        goto *dispatch_table[op]; \
    } while(0)

    #else
    #define DISPATCH() goto *dispatch_table[ADVANCE_U32()]
    #define DISPATCH_OPCODE(op) goto *dispatch_table[op]
    #endif

    #endif

   inline void _set_var_pointer_value(Value* pre, Value& post) {
        if (pre->isPointer())  pre->asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);
        *pre = post;
    }

// -----------------------------------------------------------------------------
Value runDirectThreadedVM(BytecodeChunk& chunk, U32 numLocals) {

    // TEST! register const uint8_t* pc __asm__("r14");
    if (Globals::ByteCodeRunning) {
        Tools::errorf("Sorry. use include to add code to the current byte code ... machine is running!");
        return NumberNullValue;
    }
    Globals::ByteCodeRunning = true;

    // alignas(64) constexpr void* dispatch_table[] = {
    // NOTE new X macro see also VMStructure
    #define X(name, args, comment) &&handle_##name,
    alignas(64) constexpr void* dispatch_table[] = {
        OP_CODE_LIST
    };
    #undef X


    if (chunk.mByteCodes.empty()) {
        return Value(0.0);
    }
    // -----------------------
    // stack register
    S32 _STK = 0;
    // // // Value vmStack[MAX_BC_STACK_SIZE] = {0};
    std::vector<Value>vmStack;
    vmStack.resize(MAX_BC_STACK_SIZE);

    // Benchmark optimize
    #ifdef DREIZEHN_VM_STATS
    int64_t op_counter[OP_INVALID] = {0};
    #endif

    uint32_t  ip = 0;
    uint32_t* code = chunk.mByteCodes.data();
    size_t codeSize = chunk.mByteCodes.size();

    // -----------------------
    std::vector<CallFrame> callFrame;
    callFrame.push_back({ip, 0, numLocals});
    CallFrame* curFrame = &callFrame.back();

    // std::vector<Value>locals;
    if (locals.size() == 0) locals.resize(INITIAL_BC_LOCALS_SIZE);

    // -----------------------

    DISPATCH();

    // -------------------------------
    handle_OP_CONST: {
        PUSH_STACK(chunk.mConstants[ADVANCE_U32()]);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_PUSH_VALUE: {
        // PUSH_STACK(ADVANCE_VALUE());
        PUSH_STACK(advance_value(code, ip));
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
                PrintVMError(OP_MODULO,"Modulo Division by 0.0!");
                PUSH_STACK(NumberNullValue);
            } else {
                PUSH_STACK(Value(std::fmod(lVal.asFastDouble(), r)));
            }
        } else {
            int64_t l = lVal.getInt64();
            int64_t r = rVal.getInt64();
            if (r == 0) {
                PrintVMError(OP_MODULO,"Modulo Division by 0!");
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
        // void* rawPtr = ADVANCE_PTR();
        void* rawPtr = advance_ptr(code,ip);
        Value* valuePtr = nullptr;

        if (rawPtr) {
            valuePtr = static_cast<Value*>(rawPtr);
            PUSH_STACK(*valuePtr);
            DISPATCH();
        }
        VariableFrame* frame = Globals::MasterVariableFrame;
        valuePtr = frame->getVariablePtr(symID);
        // PATCH_PTR(ptrPOS,valuePtr);
        patch_ptr(code, ptrPOS, valuePtr);
        PUSH_STACK(*valuePtr);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_VAR:{
        U32 symID = ADVANCE_U32();
        U32 ptrPOS = ip;
        // void* rawPtr = ADVANCE_PTR();
        void* rawPtr =  advance_ptr(code,ip);
        Value* valuePtr = nullptr;

        if (rawPtr) {
            valuePtr = static_cast<Value*>(rawPtr);
            *valuePtr = PEEK_STACK(0);
            DISPATCH();
        }
        VariableFrame* frame = Globals::MasterVariableFrame;
        valuePtr = frame->getVariablePtr(symID, true);
        // PATCH_PTR(ptrPOS,valuePtr);
        patch_ptr(code, ptrPOS, valuePtr);
        *valuePtr = PEEK_STACK(0);


        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Object Field
    // -------------------------------------------------------------------------
    handle_OP_LOAD_OBJECT_FIELD: {

        U32 fieldSymId = ADVANCE_U32();
        U32 saveObjPtrPos = ip;
        // void* rawObjectPtr = ADVANCE_PTR();
        void* rawObjectPtr =  advance_ptr(code,ip);
        U32 saveFieldPtrPos = ip;
        // void* rawFieldValuePtr = ADVANCE_PTR();
        void* rawFieldValuePtr =  advance_ptr(code,ip);

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            PrintVMError(OP_LOAD_OBJECT_FIELD,"Object not found:",objectValue.toString());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        Value* fieldValuePtr = nullptr;
        if ( rawObjectPtr == (void*) obj ) [[likely]] {
            fieldValuePtr = static_cast<Value*>(rawFieldValuePtr);
        } else {
            fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
            if (rawObjectPtr) {
                //we had cached once before so cache is not usable here ...
                code[saveObjPtrPos - 2] = OP_LOAD_OBJECT_FIELD_GENERIC;
            }
            if (!fieldValuePtr) [[unlikely]] {
                PrintVMError(OP_LOAD_OBJECT_FIELD, "Object have no field named:",
                             objectValue.toString(),
                             SymbolTable::getName(fieldSymId));
                PUSH_STACK(NumberNullValue);
                DISPATCH();
            }
            // PATCH_PTR(saveObjPtrPos, obj);
            patch_ptr(code, saveObjPtrPos, obj);
            // PATCH_PTR(saveFieldPtrPos, fieldValuePtr);
            patch_ptr(code, saveFieldPtrPos, fieldValuePtr);
        }
        PUSH_STACK(*fieldValuePtr);

        DISPATCH();
    }

    handle_OP_LOAD_OBJECT_FIELD_GENERIC: {

        U32 fieldSymId = ADVANCE_U32();
        SKIP_PTR();
        SKIP_PTR();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            PrintVMError(OP_LOAD_OBJECT_FIELD_GENERIC,"RunTime Error: Object not found:",objectValue.toString());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();

        Value* fieldValuePtr = nullptr;
        fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
        if (!fieldValuePtr) [[unlikely]] {
            PrintVMError(OP_LOAD_OBJECT_FIELD_GENERIC, "Object have no field named:",
                         objectValue.toString(),
                         SymbolTable::getName(fieldSymId));
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        PUSH_STACK(*fieldValuePtr);

        DISPATCH();
    }
    // -------------------------------
    handle_OP_SAVE_OBJECT_FIELD: {

        U32 fieldSymId = ADVANCE_U32();
        U32 saveObjPtrPos = ip;
        // void* rawObjectPtr = ADVANCE_PTR();
        void* rawObjectPtr =  advance_ptr(code,ip);
        U32 saveFieldPtrPos = ip;
        // void* rawFieldValuePtr = ADVANCE_PTR();
        void* rawFieldValuePtr =  advance_ptr(code,ip);

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            PrintVMError(OP_SAVE_OBJECT_FIELD, "Object not found:",objectValue.toString());
            DISPATCH();
        }

        ValueObject* obj = objectValue.asPointerObject();
        Value* fieldValuePtr = nullptr;
        if ( rawObjectPtr == (void*) obj ) [[likely]]{
            fieldValuePtr = static_cast<Value*>(rawFieldValuePtr);
        } else {
            fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
            if (rawObjectPtr) {
                //we had cached once before so cache is not usable here ...
                code[saveObjPtrPos - 2] = OP_SAVE_OBJECT_FIELD_GENERIC;
            }
            if (!fieldValuePtr) [[unlikely]] {
                PrintVMError(OP_SAVE_OBJECT_FIELD, "Object have no field named:",
                             objectValue.toString(),
                             SymbolTable::getName(fieldSymId));
                DISPATCH();
            }
            // PATCH_PTR(saveObjPtrPos, obj);
            patch_ptr(code,saveObjPtrPos, obj);
            // PATCH_PTR(saveFieldPtrPos, fieldValuePtr);
            patch_ptr(code,saveFieldPtrPos, fieldValuePtr);
        }

        _set_var_pointer_value(fieldValuePtr, PEEK_STACK(0));
        // // // *fieldValuePtr = PEEK_STACK(0);

        DISPATCH();
    }
    handle_OP_SAVE_OBJECT_FIELD_GENERIC: {

        U32 fieldSymId = ADVANCE_U32();
        SKIP_PTR();
        SKIP_PTR();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            PrintVMError(OP_SAVE_OBJECT_FIELD_GENERIC, "Object not found:",objectValue.toString());
            DISPATCH();
        }

        ValueObject* obj = objectValue.asPointerObject();
        Value* fieldValuePtr = nullptr;
        fieldValuePtr = obj->onGetFieldPtr(fieldSymId);
        if (!fieldValuePtr) [[unlikely]] {
            PrintVMError(OP_SAVE_OBJECT_FIELD_GENERIC, "Object have no field named:",
                         objectValue.toString(),
                         SymbolTable::getName(fieldSymId));
            DISPATCH();
        }

        _set_var_pointer_value(fieldValuePtr, PEEK_STACK(0));
        // // // *fieldValuePtr = PEEK_STACK(0);

        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Object methods
    // -------------------------------------------------------------------------
    handle_OP_CALL_OBJECT_METHOD : {
        // pull method name id
        U32 symID = ADVANCE_U32();
        U32 argCount = ADVANCE_U32();
        U32* sfLoopUpPtr = &code[ip];
        U32 skipSF = ADVANCE_U32();

        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) [[unlikely]] {
            PrintVMError(OP_CALL_OBJECT_METHOD, "Object not found:",objectValue.toString());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        if (!obj) [[unlikely]] {
            PrintVMError(OP_CALL_OBJECT_METHOD, "Object INVALID:",objectValue.toString());
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        {

            // holy cow crossing fingers!
            if (!skipSF) {
                FunctionMap::ScriptFunction* sf = obj->getScriptMethod(symID);
                if (sf) {
                    ip -= 4;
                    code[ ip ] = OP_CALL_SF_FUNCTION;
                    code[ ip + 1 ] = argCount + 1;
                    // PATCH_PTR( ip + 2, sf);
                    patch_ptr(code, ip + 2, sf);
                    // code[ ip + 2 ] = sf->bytecodeAddress;
                    // code[ ip + 3 ] = sf->localVarCount;
                    PUSH_STACK(obj); //push this
                    DISPATCH();
                }
                // did not find a script method next time we skip this
                *sfLoopUpPtr = 1;
            }

            std::vector<Value> evaluatedArgs(argCount);
            for (int i = (int)argCount - 1; i >= 0; i--) {
                evaluatedArgs[i] = POP_STACK();
            }

            Value returnValue = NumberNullValue;
            bool success = obj->onMethodCall(symID, evaluatedArgs, returnValue);
            if (!success) {
                PrintVMError(OP_CALL_OBJECT_METHOD,"Runtime Error in native function call\n");
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
            PrintVMError(OP_SAVE_OBJECT_METHOD, "Object not found:",objectValue.toString());
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }
        ValueObject* obj = objectValue.asPointerObject();
        if (!obj) {
            PrintVMError(OP_SAVE_OBJECT_METHOD, "Object not INVALID:",objectValue.toString());
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
            PrintVMError(OP_SAVE_OBJECT_METHOD, "Cant fins method on object:",objectValue.toString());
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }

         _set_var_pointer_value(destPrt, PEEK_STACK(0));
        // *destPrt = PEEK_STACK(0);
        DISPATCH();
    }
    // -------------------------------------------------------------------------
    // Array variable
    // -------------------------------------------------------------------------
    handle_OP_LOAD_ARRAY: {
        Value indexValue = POP_STACK();
        Value varValue = POP_STACK();

        if (!varValue.isPointer()) {
           PrintVMError(OP_LOAD_ARRAY,"Array is not an Object!");
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }
        const Value* resultPtr = nullptr;
        if (indexValue.isNumber()) [[likely]] {
             resultPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.getUInt());
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
            PrintVMError(OP_SAVE_ARRAY,"Array is not an Object!");
            PEEK_STACK(0) = NumberNullValue;
            DISPATCH();
        }
        Value* resultPtr = nullptr;
        if (indexValue.isNumber()) [[likely]] {
            resultPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.getUInt());
        } else if (indexValue.isString()) {
            resultPtr =varValue.asPointerObject()->onGetArrayByStringPtr(indexValue.getStringRef());
        }
        if (resultPtr) [[likely]]{
            // *resultPtr = assignValue;
            _set_var_pointer_value(resultPtr, assignValue);
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
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex ;
        PUSH_STACK(locals[slot]);
        DISPATCH();
    }
    // -------------------------------
    handle_OP_SET_LOCAL: {
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value* val = nullptr;

        if (slot >= curFrame->mLocalsCount) {
            curFrame->mLocalsCount++;
            if (slot  >= locals.size()) locals.resize(locals.size() * 2);
            val = &locals[slot];
        } else {
            val = &locals[slot];
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
        // void* rawPtr = ADVANCE_PTR();
        void* rawPtr =  advance_ptr(code,ip);
        FunctionMap::CallBack* callPtr = nullptr;
        if (rawPtr) [[likely]] {
            callPtr = static_cast<FunctionMap::CallBack*>(rawPtr);
        } else {
            callPtr = FunctionMap::GetCFunction(symID);
            // PATCH_PTR(ptrPOS, callPtr);
            patch_ptr(code, ptrPOS, callPtr);
        }
        U32 argCount = ADVANCE_U32();

        {
            const size_t numArgs = argCount;
            bool success = false;
            Value returnValue = NumberNullValue;

            if (numArgs <= 12) [[likely]] {
                Value evaluatedArgsStack[12];
                for (int i = (int)numArgs - 1; i >= 0; i--) {
                    evaluatedArgsStack[i] = POP_STACK();
                }
                success = (*callPtr)(std::span<Value>(evaluatedArgsStack, numArgs), returnValue);
            }
            else {
                std::vector<Value> evaluatedArgs(numArgs);
                for (int i = (int)numArgs - 1; i >= 0; i--) {
                    evaluatedArgs[i] = POP_STACK();
                }
                success = (*callPtr)(evaluatedArgs, returnValue); // Konvertiert implizit in span
            }

            // SILENT !
            // if (!success) [[unlikely]]{
                // PrintVMError(OP_CALL_C_FUNCTION, "Runtime Error in native function call\n");
            // }

            PUSH_STACK(returnValue);
        }

        DISPATCH();
    }
    // -------------------------------
    handle_OP_CALL_SF_FUNCTION: {
        U32 argCount = ADVANCE_U32();

        // void* rawPtr = ADVANCE_PTR();
        void* rawPtr =  advance_ptr(code,ip);
        // should not happen but i prevent a crash here ..

        if (!rawPtr) [[unlikely]] {
            _STK -= argCount - 1; // keep one for the function result
            DISPATCH();
        }
        FunctionMap::ScriptFunction* sf = static_cast<FunctionMap::ScriptFunction*>(rawPtr);
        U32 targetIP = sf->bytecodeAddress;
        U32 varCount = sf->localVarCount;

        // U32 targetIP = ADVANCE_U32();
        // U32 varCount = ADVANCE_U32();

        // save current ip in curFrame
        curFrame->mIP = ip;
        // push new frame with offsets:
        U32 startIndex = curFrame->mLocalsStartIndex + curFrame->mLocalsCount;
        U32 localCount = argCount + varCount;
        callFrame.push_back({targetIP, startIndex, localCount});
        // update curFrame
        curFrame = &callFrame.back();

        if (argCount + curFrame->mLocalsStartIndex  >= locals.size()) locals.resize(locals.size() * 2);


        for (int i = (int)argCount - 1; i >= 0; i--) {
            Value param = POP_STACK();
            U32 slot =  i + curFrame->mLocalsStartIndex;
            locals[slot] = param;
        }
        ip = targetIP;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_INC_LOCAL: {
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        S32 inc = ADVANCE_S32();

        Value& val = locals[slot];

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
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value b = POP_STACK();
        Value& val = locals[slot];
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
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value b = POP_STACK();
        Value& val = locals[slot];
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
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value b = POP_STACK();
        Value& val = locals[slot];
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
        U32 slot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value b = POP_STACK();
        Value& val = locals[slot];
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
    handle_OP_ADD: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
        uint64_t result_bits = std::bit_cast<uint64_t>(target_a + target_b);
        PUSH_STACK(Value(Unsafe{}, result_bits));
        DISPATCH();

//         if (a.isDouble() && b.isDouble()) [[likely]]
//         {
//             double res = a.asFastDouble() + b.asFastDouble();
//             PUSH_STACK(Value(Unsafe{}, std::bit_cast<uint64_t>(res)));
//             DISPATCH();
//         }
//         if (a.isInt() && b.isInt())
//         {
//             uint64_t res_32 = static_cast<uint32_t>(a.asFastInt() + b.asFastInt());
//             constexpr uint64_t INT_BOX_PREFIX = QNAN_MASK | TAG_INT;
//             uint64_t raw_bits = INT_BOX_PREFIX | res_32;
//             PUSH_STACK(Value(Unsafe{}, raw_bits));
//             DISPATCH();
//         }
//
//
//         PUSH_STACK(Value::handleGenericAdd(a, b));
//         DISPATCH();
    }
    // -------------------------------
    handle_OP_SUB: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
        uint64_t result_bits = std::bit_cast<uint64_t>(target_a - target_b);
        PUSH_STACK(Value(Unsafe{}, result_bits));
        DISPATCH();

        // if (a.isDouble() && b.isDouble())  [[likely]] {
        //     PUSH_STACK(Value(a.asFastDouble() - b.asFastDouble()));
        //     DISPATCH();
        // }
        // if (a.isInt() && b.isInt()) {
        //     PUSH_STACK(Value(a.asFastInt() - b.asFastInt()));
        //     DISPATCH();
        // }
        // PUSH_STACK(Value(a.getDouble() - b.getDouble()));
        // DISPATCH();
    }
    // -------------------------------
    handle_OP_MUL: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
        uint64_t result_bits = std::bit_cast<uint64_t>(target_a * target_b);
        PUSH_STACK(Value(Unsafe{}, result_bits));
        DISPATCH();

        // if (a.isDouble() && b.isDouble())  [[likely]] {
        //     PUSH_STACK(Value(a.asFastDouble() * b.asFastDouble()));
        //     DISPATCH();
        // }
        // if (a.isInt() && b.isInt()) {
        //     int32_t result;
        //     if (!__builtin_mul_overflow(a.asFastInt(), b.asFastInt(), &result)) [[likely]] {
        //         PUSH_STACK(Value(result));
        //     } else {
        //         PUSH_STACK(Value(static_cast<double>(a.asFastInt()) * static_cast<double>(b.asFastInt())));
        //     }
        //     DISPATCH();
        // }
        // PUSH_STACK(Value(a.getDouble() * b.getDouble()));
        // DISPATCH();
    }
    // -------------------------------
    handle_OP_DIV: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
        if (target_b == 0.0) [[unlikely]]  {
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }

        uint64_t result_bits = std::bit_cast<uint64_t>(target_a / target_b);
        PUSH_STACK(Value(Unsafe{}, result_bits));
        DISPATCH();

        // const bool l_is_double = a.isDouble();
        // const bool r_is_double = b.isDouble();
        // if (l_is_double && r_is_double) [[likely]] {
        //     double r = b.asFastDouble();
        //     if (r == 0.0) [[unlikely]] DISPATCH_OPCODE(OP_INVALID);;
        //     PUSH_STACK(Value(a.asFastDouble() / r));
        //     DISPATCH();
        // }
        //
        // const bool is_both_int = a.isInt() && b.isInt();
        // if (is_both_int) {
        //     int32_t r = b.asFastInt();
        //     if (r == 0) [[unlikely]] DISPATCH_OPCODE(OP_INVALID);;
        //
        //     int32_t l = a.asFastInt();
        //     if (l % r == 0) {
        //         PUSH_STACK(Value(l / r));
        //         DISPATCH();
        //     }
        //     PUSH_STACK(Value(static_cast<double>(l) / static_cast<double>(r)));
        //     DISPATCH();
        // }
        //
        // double r = b.getDouble();
        // if (r == 0.0) [[unlikely]]  DISPATCH_OPCODE(OP_INVALID);
        // PUSH_STACK(Value(a.getDouble() / r));
        // DISPATCH();
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
        U32 iterSlot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        // // get count
        Value countValue = POP_STACK();
        S32 count = countValue.getInt();
        if (count == 0) {
            ip = exitIP;
            DISPATCH();
        }

        PUSH_STACK(Value(count - 1)); // limit
        PUSH_STACK(Value(0));     // current value


        locals[iterSlot] = 0;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_STEP_INIT: {
        U32 exitIP = ADVANCE_U32();
        U32 incrementCodePos = ADVANCE_U32();
        U32 iterSlot = ADVANCE_U32() + curFrame->mLocalsStartIndex;

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
        U32 iterSlot = ADVANCE_U32() + curFrame->mLocalsStartIndex; // variable
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
            (void) POP_STACK(); // pop index
            (void) POP_STACK(); // pop limit
        }
        DISPATCH();
    }
    // -------------------------------
    handle_OP_FOR_EACH_INIT:{
        U32 exitIP = ADVANCE_U32();
        U32 iterSlot = ADVANCE_U32() + curFrame->mLocalsStartIndex;
        Value objectValue = POP_STACK();
        if (!objectValue.isPointer()) {
            PrintVMError(OP_FOR_EACH_INIT,"forEach without a object");
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

        locals[iterSlot] =  *objectValue.asPointerObject()->onGetArrayIndexPtr(0);
        DISPATCH(); //jump to body
    }
    // -------------------------------
    handle_OP_FOR_EACH_INCREMENT_JUMP:{
        U32 iterSlot = ADVANCE_U32() + curFrame->mLocalsStartIndex; // variable
        U32 targetIP = ADVANCE_U32(); // body start

        S32 currentIter = PEEK_STACK(0).asInt();
        S32 count = PEEK_STACK(1).asInt();
        ValueObject* obj = PEEK_STACK(2).asPointerObject();
        currentIter++;
        if (currentIter < count) {
             PEEK_STACK(0) = Value(currentIter);
             locals[iterSlot] =  *obj->onGetArrayIndexPtr(currentIter);
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
            PrintVMError(OP_CREATE_KEYVALUE,"KeyValueExpression value must be a string,.");
            PUSH_STACK(NumberNullValue);
            DISPATCH();
        }

        auto obj = new KeyValueObject(keyVal.getStringRef(), valueVal);

        PUSH_STACK(Value(obj));

        DISPATCH();
    }
    // -------------------------------
    handle_OP_STR_CONCAT: {
        Value b = POP_STACK();
        Value a = POP_STACK();

        PUSH_STACK(Value::handleStringAdd(a, b));
        DISPATCH();
    }
    // -------------------------------
    handle_OP_POP_STACK: {
        (void) POP_STACK();
        DISPATCH();
    }
    // -------------------------------
    handle_OP_RETURN: {

        if (callFrame.size() <  2) {
            DISPATCH_OPCODE(OP_EXIT);
        }

        Value returnValue = POP_STACK();



        if (!Globals::shutDownRUNNING) {
            U32 endIdx = curFrame->mLocalsStartIndex + curFrame->mLocalsCount;
            for (U32 i = curFrame->mLocalsStartIndex; i < endIdx; ++i) {
                auto& loc = locals[i];
                if (loc.isPointer()) {
                    loc.asPointerObject()->setAssigned(false);
                }
            }
        }
        callFrame.pop_back();
        curFrame = &callFrame.back();

        PUSH_STACK(returnValue);

        ip = curFrame->mIP;
        DISPATCH();
    }
    // -------------------------------
    handle_OP_EXIT: {
        #ifdef DEBUG_TRACE_EXECUTION
        Tools::printf("*** STACK COUNT: %zu\n", STACK_SIZE());
        #endif

        #ifdef DREIZEHN_VM_STATS
        {
            std::vector<std::pair<U32, U32>> sorted_counts;
            sorted_counts.reserve(OP_INVALID);

            for (U32 i = 0; i < OP_INVALID; i++) {
                // Tools::printf("%20s %4u\n", OpCodeInfos[i].name, op_counter[i]);
                sorted_counts.push_back({i, op_counter[i]});
            }
            Tools::printSeparator(100);
            Tools::printf(" --- SORTED ----\n");
            Tools::printSeparator(100);
            std::sort(sorted_counts.begin(), sorted_counts.end(), [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

            Tools::printf( "|OP-Code | count |\n");
            Tools::printf( "| --- | --- |\n");
            for (const auto& item : sorted_counts) {
                U32 idx = item.first;
                U32 count = item.second;
                if (count > 0) Tools::printf("|%s | %u|\n", OpCodeInfos[idx].name, count);
            }
        }
        #endif

        Globals::ByteCodeRunning = false;
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
