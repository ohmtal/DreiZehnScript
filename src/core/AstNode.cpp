//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// AST-Interpreter
//-----------------------------------------------------------------------------
#include <cmath>
#include <span>
#include "Environment.h"
#include "bytecode/CompilerScope.h"
#include "bytecode/VMStructure.h"
#include "core/FunctionMap.h"
#include <string.h>
#include "core/BaseObjects.h"

#include "core/Token.h"
#include "core/VectorFunctions.h"



namespace DreiZehn {
    // -------------------------------------------------------------------------
    void PrintASTError(const ASTNode* node,  const std::string& p1,
                       const std::string& p2 = "", const std::string& p3 = ""){

        if (!node) return;
        Tools::errorf("Runtime Error %s LINE:[%u]\n\tMessage: %s %s %s\n"
            , NodeTypeToString(node->mNodeType)
            , node->mLineNumber
            , p1.c_str()
            , p2.c_str()
            , p3.c_str()
        );


    }
    // -------------------------------------------------------------------------




    // -------------------------------------------------------------------------
    // i need this for variable lookup not found warning
    // to not show up when it's an assign
    bool AssignActive = false;
    // -------------------------------------------------------------------------


    // -------------------------------------------------------------------------
    // LiteralExpression :: optimized
    Value LiteralExpression::evaluate(Environment& env) {

        if (mEvaluated) return mEvaluatedValue;
        mEvaluatedValue = Value();

        if (mType == TokenType::Number) {
            char* endptr = nullptr;
            bool  haveDot = mRawValue.find('.') != std::string::npos;
            double resDouble = std::strtod(mRawValue.c_str(), &endptr);
            if (mRawValue.empty() || *endptr != '\0') {
                mEvaluatedValue =  NumberNullValue;
            } else {
                if (haveDot) mEvaluatedValue = Value(resDouble);
                else mEvaluatedValue =  Value((int32_t)resDouble);
            }
        }

        if (mType == TokenType::StringLiteral) {
            mEvaluatedValue = Value(mRawValue);
        }
        mEvaluated = true;
        return mEvaluatedValue;
    }

    bool LiteralExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"LiteralExpression without a request!");
            return false;
        }
        uint32_t constIdx = chunk.addConstant(this->evaluate(env));
        chunk.emit(OP_CONST);
        chunk.emit(constIdx);
        return true;
    }
    // -------------------------------------------------------------------------
    // ValueExpression
    // -------------------------------------------------------------------------
    bool ValueExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"ValueExpression without a request!");
            return false;
        }
        uint32_t constIdx = chunk.addConstant(this->evaluate(env));
        chunk.emit(OP_CONST);
        chunk.emit(constIdx);
        return true;
    }
    // -------------------------------------------------------------------------
    // VariableExpression
    // -------------------------------------------------------------------------
    Value VariableExpression::evaluate(Environment& env) {
        Value* valPtr = this->evaluatePtr(env);
        if (valPtr) [[likely]] return *valPtr;
        else  {
            std::string varName = SymbolTable::getName(mVariableNameSymbolId);
            PrintASTError(this,"Variable not found!",varName);
            return NumberNullValue;
        }

        // return env.getVariableFrame()->getVariablePtr(mVariableNameSymbolId);
    }

    Value* VariableExpression::evaluatePtr(Environment& env) {

        VariableFrame* frame = env.getVariableFrame();
        uint32_t frameID = frame->getFrameId();
        if (mCacheValue && mCacheFrameID == frameID) {
            return mCacheValue;
        }

        mCacheFrameID = frameID;
        mCacheValue =  frame->getVariablePtr(mVariableNameSymbolId, AssignActive);
        return mCacheValue;
    }

    bool VariableExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (mGlobalVariable) {
            chunk.emit(OP_LOAD_VAR);
            chunk.emit(mVariableNameSymbolId);
            chunk.emitPointer(nullptr); //space for cached ptr!
        } else {
            S32 localSlot = scope.resolve(mVariableNameSymbolId);
            if (localSlot < 0 ) {
                std::string varName = SymbolTable::getName(mVariableNameSymbolId);
                PrintASTError(this,"Variable not found in local scope", varName);
                return false; // do  stop!
            }
            chunk.emit(OP_GET_LOCAL);
            chunk.emit(localSlot);
        }
        return true;
    }

    bool VariableExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"compile pointer need a request!");
            return false;
        }
        if (mGlobalVariable) {
            chunk.emit(OP_PUSHPTR_VAR);
            chunk.emit(mVariableNameSymbolId);
            chunk.emitPointer(nullptr); //space for cache ptr
        } else {
            U32 localSlot = scope.insert(mVariableNameSymbolId);
            chunk.emit(OP_PUSHPRT_LOCAL);
            chunk.emit(static_cast<U32>(localSlot));
        }


        return true;
    }


    bool VariableExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        // save var
        if (mGlobalVariable) {
            chunk.emit(OP_SAVE_VAR);
            chunk.emit(mVariableNameSymbolId);
            chunk.emitPointer(nullptr); //space for cache ptr
        } else {
            U32 localSlot = scope.insert(mVariableNameSymbolId);
            chunk.emit(OP_SET_LOCAL);
            chunk.emit(static_cast<U32>(localSlot));
        }

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }

        return true;

    }

    // -------------------------------------------------------------------------
    // VariableReferenceExpression
    // -------------------------------------------------------------------------
    Value VariableReferenceExpression::evaluate(Environment& env) {
        if (!mVarExpr) return Value();
        Value* varValuePtr = mVarExpr->evaluatePtr(env);
        return Value(varValuePtr);
    }
    // -------------------------
    Value* VariableReferenceExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr) return nullptr;
        return mVarExpr->evaluatePtr(env);
    }

    // -------------------------
    bool VariableReferenceExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr) {
            PrintASTError(this, "bytecode compile missing Variable");
            return false;;
        }
        mVarExpr->compilePtr(env,chunk,scope,requestType);
        return true;
    }
    // -------------------------
    bool VariableReferenceExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr) {
            PrintASTError(this, "bytecode compileAssignment missing Variable");
            return false;
        }
        PrintASTError(this, "Variable Reference can't be assigned directly to set the value use (*var)");

        return false;
    }
    // -------------------------
    bool VariableReferenceExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr) {
            PrintASTError(this, "bytecode compilePtr missing Variable");
            return false;
        }
        return mVarExpr->compilePtr(env,chunk,scope,requestType);
    }

    // -------------------------------------------------------------------------
    // VariablePointerExpression
    // -------------------------------------------------------------------------
    Value VariablePointerExpression::evaluate(Environment& env) {

        Value* varValuePtr = this->evaluatePtr(env);

        if (varValuePtr) {
            return *varValuePtr;
        }
        return Value();
    }
    // -------------------------
    Value* VariablePointerExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr) return nullptr;

        if (!mCachedPtr) mCachedPtr = mVarExpr->evaluatePtr(env);

        if (mCachedPtr && mCachedPtr->isValuePointer()) {
            return mCachedPtr->asValuePointer();
        }

        return nullptr;
    }
    // -------------------------
    bool VariablePointerExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"need a request!");
            return false;
        }
        if (!mVarExpr) {
            PrintASTError(this, "bytecode compile missing Variable");
            return false;;
        }

        mVarExpr->compilePtr(env,chunk,scope,requestType);
        chunk.emit(OP_VALUE_PTR_PTR_TO_VALUE);

        return true;
    }
    // -------------------------
    bool VariablePointerExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr) {
            PrintASTError(this, "bytecode compileAssignment missing Variable");
            return false;
        }

        mVarExpr->compilePtr(env,chunk,scope,ValueType::ANY);
        chunk.emit(OP_VALUE_PTR_PTR_TO_VALUE_PTR);
        chunk.emit(OP_ASSIGN_PTR);

        if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);
        return true;
    }
    // -------------------------
    bool VariablePointerExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mVarExpr) {
            PrintASTError(this, "bytecode compilePtr missing Variable");
            return false;
        }

        mVarExpr->compilePtr(env,chunk,scope,requestType);
        chunk.emit(OP_VALUE_PTR_PTR_TO_VALUE_PTR);


        return true;
    }
    // -------------------------------------------------------------------------
    // ArrayVariableExpression
    // -------------------------------------------------------------------------
    Value ArrayVariableExpression::evaluate(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            PrintASTError(this,"Array Field invalid Variable or index");
            return NumberNullValue;
        }
        Value varValue = mVarExpr->evaluate(env);
        Value indexValue = mIndexExpr->evaluate(env);
        if (indexValue.isNumber()) {
            size_t index = (size_t)indexValue.getUInt();

            if (varValue.isString()) [[unlikely]] {
                std::string str = varValue.getStringRef();
                if (index >= str.length()) {
                    return EmptyStringValue;
                }
                std::string result(1, static_cast<char>(str[index]));
                return Value(result);
            }

            // only works on objects which support onGetArrayIndexPtr
            if (!varValue.isPointer() ) {
                return NumberNullValue;
            }
            Value* valPtr = varValue.asPointerObject()->onGetArrayIndexPtr(index);
            if (valPtr) return Value(*valPtr);
        } else if (indexValue.isString() && varValue.isPointer()) {
            //FIXME SymbolTable?!
            Value* valPtr = varValue.asPointerObject()->onGetArrayByStringSymId(SymbolTable::insert( indexValue.getStringRef()));
            if (valPtr) return Value(*valPtr);
        }

        return NumberNullValue;
    }
    // ----------------
    Value* ArrayVariableExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            PrintASTError(this,"Array Field invalid Variable or index");
            return nullptr;
        }
        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer() ) {
            return nullptr;
        }

        Value indexValue = mIndexExpr->evaluate(env);
        if (indexValue.isNumber()) {
            return varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.getUInt());
        } else if (indexValue.isString()) {
            //FIXME SymbolTable
            return varValue.asPointerObject()->onGetArrayByStringSymId(SymbolTable::insert(indexValue.getStringRef()));
        }
        // only works on objects which support onGetArrayIndexPtr
        return nullptr;
    }
    // ----------------
    bool ArrayVariableExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr || !mIndexExpr ) {
            PrintASTError(this,"Array Field invalid Variable or index");
            return false;
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::ANY)) return false;

        if (mIndexExpr->mNodeType == NodeType::LiteralExpression) {
            Value val = mIndexExpr->evaluate(env);
            if (val.isNumber()) {
                chunk.emit(OP_LOAD_ARRAY_INDEX);
                chunk.emit(val.getUInt());
            } else if (val.isString()) {
                chunk.emit(OP_LOAD_ARRAY_SYMID);
                chunk.emit(SymbolTable::insert( val.getStringRef()) );
            } else {
                PrintASTError(this,"Array Field index is NOT a number and NOT a string ?!");
                return false;
            }
        } else {
            if (!mIndexExpr->compile(env,chunk,scope,ValueType::ANY)) return false;
            chunk.emit(OP_LOAD_ARRAY);
        }


        if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);

        return true;
    }
    // ----------------
    bool ArrayVariableExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"compile pointer need a request!");
            return false;
        }

        if (!mVarExpr || !mIndexExpr ) {
            PrintASTError(this,"Array Field invalid Variable or index");
            return false;
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::ANY)) return false;

       if (mIndexExpr->mNodeType == NodeType::LiteralExpression) {
            Value val = mIndexExpr->evaluate(env);
            if (val.isNumber()) {
                chunk.emit(OP_PUSHPTR_ARRAY_INDEX);
                chunk.emit(val.getUInt());
            } else if (val.isString()) {
                chunk.emit(OP_PUSHPTR_ARRAY_SYMID);
                chunk.emit(SymbolTable::insert( val.getStringRef()) );
            } else {
                PrintASTError(this,"Array Field index is NOT a number and NOT a string ?!");
                return false;
            }
        } else {
            if (!mIndexExpr->compile(env,chunk,scope,ValueType::ANY)) return false;
            chunk.emit(OP_PUSHPTR_ARRAY);
        }

        return true;
    }
    // ----------------
    bool ArrayVariableExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr || !mIndexExpr ) {
            PrintASTError(this,"Array Field invalid Variable or index");
            return false;
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::ANY)) return false;
        if (!mIndexExpr->compile(env,chunk,scope,ValueType::ANY)) return false;

        chunk.emit(OP_SAVE_ARRAY);
        if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);
        return true;
    }

    // -------------------------------------------------------------------------
    // ObjectFieldExpression
    // -------------------------------------------------------------------------
    Value ObjectFieldExpression::evaluate(Environment& env) {

        Value returnValue = NumberNullValue;

        if (!mVarExpr) {
             PrintASTError(this,"RunTime Error expression is invalid");
             return returnValue;

        }

        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer()) {
            PrintASTError(this,"RunTime Error: Object %s not found.",varValue.toString());
            return returnValue;
        }

        ValueObject* obj = varValue.asPointerObject();
        if (obj->onGetField(mFieldSymbolId, returnValue)) {
            return returnValue;
        }
        PrintASTError(this, "Field not found! Object/Field:", varValue.toString(),SymbolTable::getName(mFieldSymbolId));
        return returnValue;
    }
    // -------
    Value* ObjectFieldExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr ) {
            PrintASTError(this, "Field without an variable");
            return nullptr;
        }
        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer()) {
            PrintASTError(this, "Object not found:",varValue.toString());
            return nullptr;
        }

        ValueObject* obj = varValue.asPointerObject();

        // cool speedup
        if (mCachedFieldValue && mCachedObject == obj) {
            return mCachedFieldValue;
        }

        mCachedFieldValue = nullptr;
        mCachedObject = nullptr;

        Value* valPtr= obj->onGetFieldPtr(mFieldSymbolId);
        if (!valPtr) {
            PrintASTError(this, "Field not found! Object/Field:", varValue.toString(),SymbolTable::getName(mFieldSymbolId));
        }
        else {
            mCachedFieldValue = valPtr;
            mCachedObject = obj;
        }
        return valPtr;
    }
    // -----------------------
    bool ObjectFieldExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr ) {
            PrintASTError(this, "Field without an variable");
            return false;
        }
        // PUSH VAR
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;

        chunk.emit(OP_LOAD_OBJECT_FIELD);
        // Field symid
        chunk.emit(mFieldSymbolId);
        // SPACE FOR Cached object
        chunk.emitPointer(nullptr);
        // SPACE FOR cached fieldvalue
        chunk.emitPointer(nullptr);

        return true;
    }
    // -----------------------
    bool ObjectFieldExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr ) {
            PrintASTError(this, "Field without an variable");
            return false;
        }
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"compile pointer need a request!");
            return false;
        }
        // PUSH VAR
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;

        chunk.emit(OP_PUSHPTR_OBJECT_FIELD);
        // Field symid
        chunk.emit(mFieldSymbolId);
        // SPACE FOR Cached object
        chunk.emitPointer(nullptr);
        // SPACE FOR cached fieldvalue
        chunk.emitPointer(nullptr);


        return true;
    }
    // -----------------------
    bool ObjectFieldExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mVarExpr ) {
            PrintASTError(this, "Field without an variable");
            return false;
        }
        // PUSH VAR
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;

        chunk.emit(OP_SAVE_OBJECT_FIELD);
        // Field symid
        chunk.emit(mFieldSymbolId);
        // SPACE FOR Cached object
        chunk.emitPointer(nullptr);
        // SPACE FOR cached fieldvalue
        chunk.emitPointer(nullptr);

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }

        return true;
    }
    // -------------------------------------------------------------------------
    // MethodExpression
    // -------------------------------------------------------------------------
    Value MethodExpression::evaluate(Environment& env) {

        if (!mVarExpr) {
            PrintASTError(this, "Variable Expression is invalid");
            return NumberNullValue;

        }
        Value objectPointer = mVarExpr->evaluate(env);
        if (!objectPointer.isPointer()) {
            PrintASTError(this, "Object not found:", objectPointer.toString());
            return NumberNullValue;
        }
        ValueObject* obj = objectPointer.asPointerObject();
        if (!obj) {
            PrintASTError(this, "Invalid Object:", objectPointer.toString());
            return NumberNullValue;
        }
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : mArguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        Value returnValue = NumberNullValue;

        obj->onMethodCall(mMethodNameSymbolId, evaluatedArgs, returnValue);
        return returnValue;

    }
    // ---------------------
    Value* MethodExpression::evaluatePtr(Environment& env) {
        Value objectPointer = mVarExpr->evaluate(env);
        if (!objectPointer.isPointer()) {
           PrintASTError(this, "Object not found:", objectPointer.toString());
            return nullptr;
        }
        ValueObject* obj = objectPointer.asPointerObject();
        if (!obj) {
            PrintASTError(this, "Object not invalid:", objectPointer.toString());
            return nullptr;
        }
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : mArguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        return obj->onMethodCallGetAssignPtr(mMethodNameSymbolId, evaluatedArgs);
    }
    // ---------------------
    bool MethodExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mVarExpr) {
            PrintASTError(this, "No Variable Expression!");
            return false;
        }
        U32 argCount = 0;
        for (auto& argExpr : mArguments) {
            if (argExpr) {
                if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                argCount++;
            }
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;
        chunk.emit(OP_CALL_OBJECT_METHOD);
        chunk.emit(mMethodNameSymbolId);
        chunk.emit(argCount);
        chunk.emit(0); //placeholder when it's a Script method

        if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);
        return true;
    }
    // ---------------------
    bool MethodExpression::compilePtr(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"compile pointer need a request!");
            return false;
        }
        if (!mVarExpr) {
            PrintASTError(this, "No Variable Expression!");
            return false;
        }
        U32 argCount = 0;
        for (auto& argExpr : mArguments) {
            if (argExpr) {
                if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                argCount++;
            }
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;
        chunk.emit(OP_PUSHPTR_OBJECT_METHOD);
        chunk.emit(mMethodNameSymbolId);
        chunk.emit(argCount);

        return true;
    }
    // ---------------------
    bool MethodExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mVarExpr) {
            PrintASTError(this, "No Variable Expression!");
            return false;
        }
        U32 argCount = 0;
        for (auto& argExpr : mArguments) {
            if (argExpr) {
                if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                argCount++;
            }
        }
        if (!mVarExpr->compile(env,chunk,scope,ValueType::Pointer)) return false;
        chunk.emit(OP_SAVE_OBJECT_METHOD);
        chunk.emit(mMethodNameSymbolId);
        chunk.emit(argCount);

        if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);
        return true;
    }
    // -------------------------------------------------------------------------
    // ListExpression
    // -------------------------------------------------------------------------
    Value ListExpression::evaluate(Environment& env) {

        VectorValueObject* arr = new VectorValueObject();

        // add args!
        for (auto& argExpr: arguments) {
            if (!argExpr) continue;
            auto arg = argExpr->evaluate(env);

            if (arg.isPointer()) {
                arg.asPointerObject()->setAssigned(true);
            }
            arr->mElements.push_back(arg);
        }

        return Value(arr);
    }
    // --------------------
    bool ListExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"list need a request!");
            return false;
        }
        U32 argCount = 0;
        for (auto& argExpr : arguments) {
            if (argExpr) {
                if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                argCount++;
            }
        }
        chunk.emit(OP_CREATE_LIST);
        chunk.emit(argCount);

        return true;
    }
    // -------------------------------------------------------------------------
    // CallExpression
    // -------------------------------------------------------------------------

    Value CallFunc(Environment& env, const FunctionMap::CallBack* cb
    , std::vector<std::unique_ptr<Expression>>& arguments, uint32_t symid) {

        const size_t numArgs = arguments.size();

        if (numArgs <= 12) {
            Value evaluatedArgsStack[12];

            for (size_t i = 0; i < numArgs; ++i) {
                if (arguments[i]) {
                    evaluatedArgsStack[i] = arguments[i]->evaluate(env);
                }
            }
            Value returnValue = NumberNullValue;
            bool success = (*cb)(std::span<Value>(evaluatedArgsStack, numArgs), returnValue);

            if (!success) [[unlikely]] {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
            }
            return returnValue;
        }
        else {
            std::vector<Value> evaluatedArgs;
            evaluatedArgs.reserve(numArgs);
            for (auto& argExpr : arguments) {
                if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
            }

            Value returnValue = NumberNullValue;
            bool success = (*cb)(evaluatedArgs, returnValue); // bzw. std::span(evaluatedArgs)

            if (!success) [[unlikely]] {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
            }
            return returnValue;
        }
    }


    Value CallScriptFunc(Environment& env, const FunctionMap::ScriptFunction* sf
    , std::vector<std::unique_ptr<Expression>>& arguments) {
        auto& func = *(sf);
        Environment localEnv(&env);

        for (size_t i = 0; i < func.parameterNames.size(); ++i) {
            if (i < arguments.size()) {
                Value evaluatedArg = arguments[i]->evaluate(env);
                localEnv.getVariableFrame()->setVariable(SymbolTable::insert(func.parameterNames[i]), evaluatedArg, true);
            } else {
                // 0.8d fill up arguments with 0
                localEnv.getVariableFrame()->setVariable(SymbolTable::insert(func.parameterNames[i]), NumberNullValue, true);
            }
        }
        Value functionResult = NumberNullValue;
        for (auto& statement : func.body) {
            FlowStatus status = env.execute(statement.get(), localEnv);

            if (status.signal == FlowSignal::Return) {
                return status.returnValue;
            }
        }

        return functionResult;
    }


    // -------------------------------------------------------------------------
    Value CallExpression::evaluate(Environment& env) {

        if (cbCache) return CallFunc(env,cbCache,arguments,mFuncSymbolId);
        if (sfCache) return CallScriptFunc(env,sfCache, arguments);

        cbCache = FunctionMap::GetCFunction(mFuncSymbolId);
        if (cbCache) return CallFunc(env,cbCache,arguments,mFuncSymbolId);

        sfCache =  FunctionMap::GetScriptFunction(mFuncSymbolId);
        if (sfCache) return CallScriptFunc(env,sfCache, arguments);

        PrintASTError(this, "Unknown command:", SymbolTable::getName(mFuncSymbolId));
        return Value();

    }

    bool CallExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {

        FunctionMap::CallBack* cb = FunctionMap::GetCFunction(mFuncSymbolId);
        if (cb) {
            U32 argCount = 0;
            for (auto& argExpr : arguments) {
                if (argExpr) {
                    if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                    argCount++;
                }
            }
            chunk.emit(OP_CALL_C_FUNCTION);
            chunk.emit(mFuncSymbolId);

            if (env.mIsByteCodeCodelet) {
                chunk.emitPointer(cb); // we use a codelet patch in the pointer directly
            } else {
                mFunctionPatches.push_back({cb,chunk.tell(),FuncPatchType::CFuncPrt});
                chunk.emitPointer(nullptr); //placeholder for cache
            }
            chunk.emit(argCount);


            if (requestType == ValueType::NONE) {
                chunk.emit(OP_POP_STACK);
            }
            return true;
        }
        FunctionMap::ScriptFunction* sf = FunctionMap::GetScriptFunction(mFuncSymbolId);
        if (sf) {
            U32 argCount = 0;
            for (auto& argExpr : arguments) {
                if (argExpr) {
                    if (!argExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
                    argCount++;
                }
            }

            chunk.emit(OP_CALL_SF_FUNCTION);

            chunk.emit(argCount);

            // mFunctionPatches.push_back({sf, chunk.tell(), FuncPatchType::Adress});
            // chunk.emit(0);
            // mFunctionPatches.push_back({sf, chunk.tell(), FuncPatchType::VarCount});
            // chunk.emit(0);

            if (env.mIsByteCodeCodelet) {

                chunk.emitPointer(sf);
            } else {
                //  0.9f changed to script pointer
                mFunctionPatches.push_back({sf, chunk.tell(), FuncPatchType::ScriptFuncPtr});
                chunk.emitPointer(nullptr);
            }



            if (requestType == ValueType::NONE) {
                chunk.emit(OP_POP_STACK);
            }
            return true;
        }



        PrintASTError(this, "Unknown command:", SymbolTable::getName(mFuncSymbolId));
        return false;
    }

    // -------------------------------------------------------------------------
    // BinaryOpExpression
    // -------------------------------------------------------------------------
    Value BinaryOpExpression::evaluate(Environment& env) {
        if (!mLeft || !mRight) [[unlikely]] {
            PrintASTError(this, "left or right is missing.");
            return Value();
        }

        Value a = evaluateFastPath(mLeft.get(),env);
        Value b = evaluateFastPath(mRight.get(),env);

        // Value a = mLeft->evaluate(env);
        // Value b = mRight->evaluate(env);

        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);


        switch (mOp) {
            case TokenType::Plus: {
                 // return Value(target_a + target_b);
                uint64_t result_bits = std::bit_cast<uint64_t>(target_a + target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::Minus: {
                 // return Value(target_a - target_b);
                uint64_t result_bits = std::bit_cast<uint64_t>(target_a - target_b);
                return Value(Unsafe{}, result_bits);
            }

            case TokenType::Mul: {
                 // return Value(target_a * target_b);
                uint64_t result_bits = std::bit_cast<uint64_t>(target_a * target_b);
                return Value(Unsafe{}, result_bits);
            }

            case TokenType::Div: {
                 if (target_b == 0.0) [[unlikely]] return NumberNullValue;
                 // return Value(target_a / target_b);
                uint64_t result_bits = std::bit_cast<uint64_t>(target_a / target_b);
                return Value(Unsafe{}, result_bits);
            }

            default:
                return Value();
        }
    }
    // Value BinaryOpExpression::evaluate(Environment& env) {
    //     if (!mLeft || !mRight) [[unlikely]] {
    //         Tools::PrintParseError("left or right is missing:");
    //         return Value();
    //     }
    //
    //     Value lVal = mLeft->evaluate(env);
    //     Value rVal = mRight->evaluate(env);
    //
    //     const bool l_is_double = lVal.isDouble();
    //     const bool r_is_double = rVal.isDouble();
    //     const bool is_both_int = lVal.isInt() && rVal.isInt();
    //
    //     switch (mOp) {
    //         case TokenType::Plus: {
    //             if (l_is_double && r_is_double) [[likely]] {
    //                 return Value(lVal.asFastDouble() + rVal.asFastDouble());
    //             }
    //             if (is_both_int) {
    //                 return Value(lVal.asFastInt() + rVal.asFastInt());
    //             }
    //             return Value::handleGenericAdd(lVal, rVal);
    //         }
    //
    //         case TokenType::Minus: {
    //             if (l_is_double && r_is_double) [[likely]] {
    //                 return Value(lVal.asFastDouble() - rVal.asFastDouble());
    //             }
    //             if (is_both_int) {
    //                 return Value(lVal.asFastInt() - rVal.asFastInt());
    //             }
    //             // Fallback für gemischte Numerik (Int - Double)
    //             return Value(lVal.getDouble() - rVal.getDouble());
    //         }
    //
    //         case TokenType::Mul: {
    //             if (l_is_double && r_is_double) [[likely]] {
    //                 return Value(lVal.asFastDouble() * rVal.asFastDouble());
    //             }
    //             if (is_both_int) {
    //                 return Value(lVal.asFastInt() * rVal.asFastInt());
    //             }
    //             return Value(lVal.getDouble() * rVal.getDouble());
    //         }
    //
    //         case TokenType::Div: {
    //             if (l_is_double && r_is_double) [[likely]] {
    //                 double r = rVal.asFastDouble();
    //                 if (r == 0.0) [[unlikely]] return NumberNullValue;
    //                 return Value(lVal.asFastDouble() / r);
    //             }
    //             if (is_both_int) {
    //                 int32_t r = rVal.asFastInt();
    //                 if (r == 0) [[unlikely]] return NumberNullValue;
    //
    //                 int32_t l = lVal.asFastInt();
    //                 if (l % r == 0) return Value(l / r);
    //                 return Value(static_cast<double>(l) / static_cast<double>(r));
    //             }
    //
    //             double r = rVal.getDouble();
    //             if (r == 0.0) [[unlikely]] return NumberNullValue;
    //             return Value(lVal.getDouble() / r);
    //         }
    //
    //         default:
    //             return Value();
    //     }
    // }

    // --------
    bool BinaryOpExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        // bool unary = false;
        if (requestType == ValueType::NONE) {
            // if (mOp != TokenType::Minus) {
                PrintASTError(this, "Invalid calculation: If you want to add a '-' Value encapsulate it like (-1).");
                return false;
            // }
            // unary hack
            // unary = true;
        }


        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this, "left or right is missing");
            return false;
        }

        if (!this->mLeft->compile(env,chunk, scope, requestType)) return false;
        if (!this->mRight->compile(env,chunk, scope, requestType)) return false;


        switch (this->mOp) {
            case TokenType::Plus    : chunk.emit(OP_ADD); break;
            case TokenType::Minus   : chunk.emit(OP_SUB); break;
            case TokenType::Mul     : chunk.emit(OP_MUL); break;
            case TokenType::Div     : chunk.emit(OP_DIV); break;
            default:
                PrintASTError(this, "COMPILE ERROR BinaryOpExpression : unknown TokenType:", std::to_string( (int)this->mOp) );
                return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    // ++ --
    Value BinaryInlineExpression::evaluate(Environment& env) {
        Value* valPtr = mExpr->evaluatePtr(env);

        if (!valPtr) [[unlikely]] {
            PrintASTError(this,"variable is missing or invalid");
            return Value();
        }

        if (valPtr->isInt()) [[likely]] {
            int32_t val = valPtr->asFastInt();
            if (mOp == TokenType::PlusPlus) {
                val++;
            } else if (mOp == TokenType::MinusMinus) {
                val--;
            }
            *valPtr = Value(val);
            return *valPtr;
        }

        if (valPtr->isDouble()) {
            double val = valPtr->asFastDouble();
            if (mOp == TokenType::PlusPlus) {
                val += 1.0;
            } else if (mOp == TokenType::MinusMinus) {
                val -= 1.0;
            }
            *valPtr = Value(val);
            return *valPtr;
        }

        PrintASTError(this,"Invalid type for increment/decrement operator!");
        return Value();
    }

    // Value BinaryInlineExpression::evaluate(Environment& env)  {
    //
    //     Value * valPtr = mExpr->evaluatePtr(env);
    //
    //
    //     if ( !valPtr  ) {
    //         Tools::PrintParseError("variable is missing or invalid");
    //         return Value();
    //     }
    //
    //     if (valPtr->isInt() ) {
    //         int32_t lCurInt = valPtr->asInt();
    //         switch (mOp) {
    //             case TokenType::PlusPlus: lCurInt++; break;
    //             case TokenType::MinusMinus: lCurInt--; break;
    //             default: break;
    //         }
    //         *valPtr = Value(lCurInt);
    //         return *valPtr;
    //     } else  { // must be a Double!
    //         double lCurDouble = valPtr->getDouble();
    //         switch (mOp) {
    //             case TokenType::PlusPlus: lCurDouble += 1.0 ; break;
    //             case TokenType::MinusMinus: lCurDouble -= 1.0 ; break;
    //             default: break;
    //         }
    //         *valPtr = Value(lCurDouble);
    //         return *valPtr;
    //     }
    //
    //
    //     return Value();
    //
    // }
    // ---------------------
    bool BinaryInlineExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mExpr) return false;

        S32 inc = (mOp == TokenType::PlusPlus) ? 1 : - 1;


        // 1. i need to know if it's a local
        if (mExpr->mNodeType == NodeType::VariableExpression) {
            VariableExpression* vExpr = static_cast<VariableExpression*>(mExpr.get());
            if (!vExpr) return false;
            if (!vExpr->mGlobalVariable) {
                S32 localSlot = scope.resolve(vExpr->mVariableNameSymbolId);
                if (localSlot < 0 ) {
                    std::string varName = SymbolTable::getName(vExpr->mVariableNameSymbolId);
                    PrintASTError(this, "Variable not found in local scope:", varName);
                    return true; // do not stop!
                }

                chunk.emit(OP_INC_LOCAL);
                chunk.emit(localSlot);
                chunk.emitS32(inc);

                if (requestType == ValueType::NONE) {
                    chunk.emit(OP_POP_STACK);
                }
                return true;
            }
        }

        // load var pointer
        // FIXME FALLBACK
        if (mExpr->compilePtr(env,chunk,scope,ValueType::ANY)) {
            chunk.emit(OP_INC_PTR);
            chunk.emitS32(inc);
            if (requestType == ValueType::NONE) {
                chunk.emit(OP_POP_STACK);
            }
            return true;
        } else {

            // load left
            if (!mExpr->compile(env,chunk,scope,ValueType::ANY)) return false;
            // PUSH INC
            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(Value(inc));
            // ADD INC
            chunk.emit(OP_ADD);
            // SAVE VAR
            if (!mExpr->compileAssignment(env,chunk,scope, requestType)) return false;
            return true;
        }




    }
    // -------------------------------------------------------------------------
    Value BinaryAndOrExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this,"(&& ||) left or right is missing");
            return Value();
        }

        if (mOp == TokenType::And) {
            // Value a = evaluateFastPath(mLeft.get(),env);
            Value a = mLeft->evaluate(env);
            U32 l = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : a.getUInt());
            if (l == 0) return Value(0);
            // Value b = evaluateFastPath(mRight.get(),env);
            Value b = mRight->evaluate(env);
            U32 r = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : b.getUInt());
            return Value( l && r);
        } else
        if (mOp == TokenType::Or) {
            // Value a = evaluateFastPath(mLeft.get(),env);
            Value a = mLeft->evaluate(env);
            U32 l = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : a.getUInt());
            if (l == 1) return Value(1);
            // Value b = evaluateFastPath(mRight.get(),env);
            Value b = mRight->evaluate(env);
            U32 r = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : b.getUInt());
            return Value( l || r);
        }

        return NumberNullValue;

    }
    // -------------------------------
    bool BinaryAndOrExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this,"(&& ||) left or right is missing");
            return false;
        }

        if (!mLeft->compile(env,chunk,scope, ValueType::ANY)) return false;

        if (mOp == TokenType::Or) {
            chunk.emit(OP_JUMP_IF_TRUE_NO_POP);
            U32 jumpOffset = chunk.tell();
            chunk.emit(0); //placeholder

            chunk.emit(OP_POP_STACK);
            if (!mRight->compile(env,chunk,scope, ValueType::ANY)) return false;
            chunk.patch(jumpOffset, chunk.tell());

        } else { // TokenType::And
            chunk.emit(OP_JUMP_IF_FALSE_NO_POP);
            U32 jumpOffset = chunk.tell();
            chunk.emit(0); // placeholder

            chunk.emit(OP_POP_STACK);
            if (!mRight->compile(env, chunk, scope, ValueType::ANY)) return false;

            chunk.patch(jumpOffset, chunk.tell());
        }

        if (requestType == ValueType::NONE)
             chunk.emit(OP_POP_STACK);

        return true;
    }
    // -------------------------------------------------------------------------
    bool BinaryExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this,"left or right is missing");
            return false;
        }

        if (!mLeft->compile(env, chunk, scope, ValueType::ANY)) return false;
        if (!mRight->compile(env, chunk, scope, ValueType::ANY)) return false;

        switch (mOp) {
            case TokenType::Greater:      chunk.emit(OP_GREATER); break;
            case TokenType::GreaterEqual: chunk.emit(OP_GREATER_EQUAL); break;
            case TokenType::Less:         chunk.emit(OP_LESS); break;
            case TokenType::LowerEqual:   chunk.emit(OP_LESS_EQUAL); break;
            case TokenType::Equal:        chunk.emit(OP_EQUAL); break;
            case TokenType::NotEqual:     chunk.emit(OP_NOT_EQUAL); break;

            case TokenType::BitAnd:       chunk.emit(OP_BIT_AND); break;
            case TokenType::BitOr:        chunk.emit(OP_BIT_OR); break;
            case TokenType::BitXOr:       chunk.emit(OP_BIT_XOR); break;
            case TokenType::SHL:          chunk.emit(OP_SHL); break;
            case TokenType::SHR:          chunk.emit(OP_SHR); break;
            case TokenType::Modulo:       chunk.emit(OP_MODULO); break;

            default:
                PrintASTError(this,"Unknown binary operator");
                return false;
        }

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }

        return true;
    }
    // -------------------------------------------------------------------------
    // BinaryExpression::evaluate
    // -------------------------------------------------------------------------
    Value BinaryExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this,"left or right is missing");
            return Value();
        }

        Value a = evaluateFastPath( mLeft.get(),env);
        Value b = evaluateFastPath( mRight.get(),env);


        switch (mOp) {
            case TokenType::Greater: {
                double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a > target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::GreaterEqual: {
                double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a >= target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::Less:  {
                double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a < target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::LowerEqual: {
                double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a <= target_b);
                return Value(Unsafe{}, result_bits);

            }
            case TokenType::Equal: {

                if (a.isNumber() && b.isNumber()) [[likely]]{
                    double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                    double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                    uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a == target_b);
                    return Value(Unsafe{}, result_bits);
                }
                else [[unlikely]] {

                    if (a.isPointer() && b.isPointer())  {
                        return Value(a.asPointer() == b.asPointer() ? 1 : 0);
                    } else if (a.isString() && b.isString()) {
                        return Value(a.getStringRef() == b.getStringRef());
                    } else if (a.isString() || b.isString()) {
                        return NumberNullValue;
                    }
                }
            }
            case TokenType::NotEqual: {
                if (a.isNumber() && b.isNumber()) [[likely]]{
                    double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
                    double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);
                    uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a != target_b);
                    return Value(Unsafe{}, result_bits);
                }
                else [[unlikely]] {

                    if (a.isPointer() && b.isPointer())  {
                        return Value(a.asPointer() == b.asPointer() ? 0 : 1);
                    } else if (a.isString() && b.isString()) {
                        return Value(a.getStringRef() != b.getStringRef());
                    } else if (a.isString() || b.isString()) {
                        return Value(1);
                    }
                }
            }

            case TokenType::BitAnd: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a & target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::BitOr: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a | target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::BitXOr: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a ^ target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::SHL: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a << target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::SHR: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a >> target_b);
                return Value(Unsafe{}, result_bits);
            }
            case TokenType::Modulo: {
                U32 target_a = a.isInt() ? a.asFastInt() : (a.isDouble() ? static_cast<U32>(a.asFastDouble()) : 0);
                U32 target_b = b.isInt() ? b.asFastInt() : (b.isDouble() ? static_cast<U32>(b.asFastDouble()) : 0);

                uint64_t result_bits = QNAN_MASK | TAG_INT | (target_a % target_b);
                return Value(Unsafe{}, result_bits);

            }
            default: break;
        }
        return NumberNullValue;
    }
    // Value BinaryExpression::evaluate(Environment& env)  {
    //     if (!mLeft.get() || !mRight.get()) {
    //         Tools::PrintParseError("left or right is missing:");
    //         return Value();
    //     }
    //
    //
    //     Value lVal = evaluateFastPath( mLeft.get(),env);
    //     Value rVal = evaluateFastPath( mRight.get(),env);
    //
    //
    //     // Value lVal = mLeft->evaluate(env);
    //     // Value rVal = mRight->evaluate(env);
    //
    //     switch (mOp) {
    //         case TokenType::Greater: {
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value((l - r) > EPSILON ? 1 : 0);
    //         }
    //         case TokenType::GreaterEqual: {
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value(l > (r - EPSILON) ? 1 : 0);
    //         }
    //         case TokenType::Less:  {
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value((r - l) > EPSILON ? 1 : 0);
    //         }
    //         case TokenType::LowerEqual: {
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value(l < (r + EPSILON) ? 1 : 0);
    //         }
    //         case TokenType::Equal: {
    //             if (lVal.isPointer() && rVal.isPointer()) {
    //                 return Value(lVal.asPointer() == rVal.asPointer() ? 1 : 0);
    //             } else if (lVal.isString() && rVal.isString()) {
    //                 return strcmp(lVal.getStringRef().c_str(), rVal.getStringRef().c_str()) == 0;
    //             } else if (lVal.isString() || rVal.isString()) {
    //                 return NumberNullValue;
    //             }
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value(std::abs(l - r) < EPSILON ? 1 : 0);
    //         }
    //         case TokenType::NotEqual: {
    //             if (lVal.isPointer() && rVal.isPointer()) {
    //                 return Value(lVal.asPointer() != rVal.asPointer() ? 1 : 0);
    //             } else if (lVal.isString() && rVal.isString()) {
    //                 return strcmp(lVal.getStringRef().c_str(), rVal.getStringRef().c_str()) != 0;
    //             }  else if (lVal.isString() || rVal.isString()) {
    //                 return Value(1);
    //             }
    //             double l = lVal.getDouble();
    //             double r = rVal.getDouble();
    //             return Value(std::abs(l - r) < EPSILON ? 0 : 1);
    //         }
    //
    //         case TokenType::BitAnd: {
    //             int l = lVal.getUInt();
    //             int r = rVal.getUInt();
    //             return Value( l & r);
    //         }
    //         case TokenType::BitOr: {
    //             int l = lVal.getUInt();
    //             int r = rVal.getUInt();
    //             return Value( l | r);
    //         }
    //         case TokenType::BitXOr: {
    //             int l = lVal.getUInt();
    //             int r = rVal.getUInt();
    //             return Value( l ^ r);
    //         }
    //         case TokenType::SHL: {
    //             int l = lVal.getUInt();
    //             int r = rVal.getUInt();
    //             return Value( l << r);
    //         }
    //         case TokenType::SHR: {
    //             int l = lVal.getUInt();
    //             int r = rVal.getUInt();
    //             return Value( l >> r);
    //         }
    //         case TokenType::Modulo: {
    //
    //             if (lVal.isDouble() && rVal.isDouble() ) {
    //                 double r = rVal.asFastDouble();
    //                 if (r == 0.0) {
    //                     Tools::PrintRuntimeError("Modulo Division by 0.0!");
    //                     return NumberNullValue;
    //                 }
    //                 return Value(std::fmod(lVal.asFastDouble(),rVal.asFastDouble()));
    //             } else {
    //                 int64_t l = lVal.getInt64();
    //                 int64_t r = rVal.getInt64();
    //                 if (r == 0) {
    //                     Tools::PrintRuntimeError("Modulo Division by 0!");
    //                     return NumberNullValue;
    //                 }
    //                 int64_t res = l % r;
    //
    //                 return Value(static_cast<uint32_t>(res));
    //
    //             }
    //         }
    //         default: break;
    //     }
    //     return NumberNullValue;
    // }


    // -------------------------------------------------------------------------

    Value BinarySingleRightOpExpression::evaluate(Environment& env)  {
        if (!mRight) {
            PrintASTError(this,"Not missing right.");
            return NumberNullValue;
        }
        Value rVal = mRight->evaluate(env);
        return Value(!rVal.getInt());
    }
    bool BinarySingleRightOpExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"Expression without a request!");
            return false;
        }
        if (!mRight) return false;

        if (!mRight->compile(env,chunk,scope,requestType)) return false;
        chunk.emit(OP_NOT);

        return true;
    }

    // -------------------------------------------------------------------------
    // STATEMENTS execute
    // -------------------------------------------------------------------------
    Value AssignStatement::evaluate(Environment& env) {

        AssignActive = true;
        Value * valPtr = mVarExpr->evaluatePtr(env);
        AssignActive = false;

        if ( !valPtr  ) {
            PrintASTError(this,"Assign variable is missing or invalid");
            return NumberNullValue;
        }

        // since i set it with pointer i need to check assignment here !!!!!!
        Value& pre = *valPtr;
        Value post = this->mRhs->evaluate(env);
        if (pre.isPointer())  pre.asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);

        *valPtr = post;

        return *valPtr;

    }
    bool AssignStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {

         if (!mVarExpr || !mRhs) [[unlikely]] {
             PrintASTError(this,"AssignOP: variable or right is missing or invalid");
             return false;
         }
        // types must support compileAssignment!
        if (!mRhs->compile(env,chunk,scope,ValueType::ANY)) return false;
        return mVarExpr->compileAssignment(env, chunk, scope, requestType);

    }

    // -------------------------------------------------------------------------
    Value AssignOPStatement::evaluate(Environment& env) {
        if (!mVarExpr) [[unlikely]] {
            PrintASTError(this,"AssignOP: variable is missing or invalid");
            return NumberNullValue;
        }
        Value* valuePtr = mVarExpr->evaluatePtr(env);

        if (!valuePtr || !mRhs) [[unlikely]] {
            PrintASTError(this,"AssignOP: variable or right is missing or invalid");
            return NumberNullValue;
        }

        Value rightHand = mRhs->evaluate(env);

        // ---------------
        // NOTE THIS IS NOT FASTER
        // double target_a = valuePtr->isDouble() ? valuePtr->asFastDouble() : (valuePtr->isInt() ? static_cast<double>(valuePtr->asFastInt()) : 0.0);
        // double target_b = rightHand.isDouble() ? rightHand.asFastDouble() : (rightHand.isInt() ? static_cast<double>(rightHand.asFastInt()) : 0.0);
        //
        // switch(mOp) {
        //     case TokenType::AssignPlus:  target_a += target_b;  break;
        //     case TokenType::AssignMinus: target_a -= target_b;  break;
        //     case TokenType::AssignMul:   target_a *= target_b;  break;
        //     case TokenType::AssignDiv:   target_a /= target_b;  break;
        //     default: return false;
        // }
        //
        // *valuePtr = target_a;
        // return *valuePtr;

        // ---------------
        Value result;
        const bool l_is_double = valuePtr->isDouble();
        const bool r_is_double = rightHand.isDouble();
        const bool is_both_int = valuePtr->isInt() && rightHand.isInt();

        switch (mOp) {
            case TokenType::AssignPlus: {
                if (l_is_double && r_is_double) [[likely]] result = Value(valuePtr->asFastDouble() + rightHand.asFastDouble());
                else if (is_both_int)                      result = Value(valuePtr->asFastInt() + rightHand.asFastInt());
                else                                       result = Value(valuePtr->getDouble() + rightHand.getDouble());
                break;
            }
            case TokenType::AssignMinus: {
                if (l_is_double && r_is_double) [[likely]] result = Value(valuePtr->asFastDouble() - rightHand.asFastDouble());
                else if (is_both_int)                      result = Value(valuePtr->asFastInt() - rightHand.asFastInt());
                else                                       result = Value(valuePtr->getDouble() - rightHand.getDouble());
                break;
            }
            case TokenType::AssignMul: {
                if (l_is_double && r_is_double) [[likely]] result = Value(valuePtr->asFastDouble() * rightHand.asFastDouble());
                else if (is_both_int)                      result = Value(valuePtr->asFastInt() * rightHand.asFastInt());
                else                                       result = Value(valuePtr->getDouble() * rightHand.getDouble());
                break;
            }
            case TokenType::AssignDiv: {
                if (l_is_double && r_is_double) [[likely]] {
                    double r = rightHand.asFastDouble();
                    result = (r == 0.0) ? NumberNullValue : Value(valuePtr->asFastDouble() / r);
                } else if (is_both_int) {
                    int32_t r = rightHand.asFastInt();
                    if (r == 0) [[unlikely]] return NumberNullValue;
                    int32_t l = valuePtr->asFastInt();
                    result = (l % r == 0) ? Value(l / r) : Value(static_cast<double>(l) / static_cast<double>(r));
                } else {
                    double r = rightHand.getDouble();
                    result = (r == 0.0) ? NumberNullValue : Value(valuePtr->getDouble() / r);
                }
                break;
            }
            default:
                return NumberNullValue;
        }

        *valuePtr = result;
        return *valuePtr;
    }

    // ---------------------
    bool AssignOPStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mVarExpr || !mRhs) [[unlikely]] {
            PrintASTError(this,"AssignOP: variable or right is missing or invalid");
            return false;
        }

        if (mVarExpr->mNodeType == NodeType::VariableExpression) {
            VariableExpression* vExpr = static_cast<VariableExpression*>(mVarExpr.get());
            if (!vExpr->mGlobalVariable) {
                if (!mRhs->compile(env,chunk,scope,ValueType::ANY)) return false;
                S32 localSlot = scope.resolve(vExpr->mVariableNameSymbolId);
                if (localSlot < 0 ) {
                    std::string varName = SymbolTable::getName(vExpr->mVariableNameSymbolId);
                    PrintASTError(this,"Variable not found :", varName);
                    return true; // do not stop!
                }

                switch(mOp) {
                    case TokenType::AssignPlus:  chunk.emit(OP_ADD_LOCAL);  break;
                    case TokenType::AssignMinus: chunk.emit(OP_SUB_LOCAL);  break;
                    case TokenType::AssignMul:   chunk.emit(OP_MUL_LOCAL);  break;
                    case TokenType::AssignDiv:   chunk.emit(OP_DIV_LOCAL);  break;
                    default: return false;
                }

                chunk.emit(localSlot);

                if (requestType == ValueType::NONE) {
                    chunk.emit(OP_POP_STACK);
                }
                return true;
            }
        }

        if (mVarExpr->compilePtr(env,chunk,scope,ValueType::ANY)) {
            if (!mRhs->compile(env,chunk,scope,ValueType::ANY)) return false;

            switch(mOp) {
                case TokenType::AssignPlus:  chunk.emit(OP_ADD_PTR);  break;
                case TokenType::AssignMinus: chunk.emit(OP_SUB_PTR);  break;
                case TokenType::AssignMul:   chunk.emit(OP_MUL_PTR);  break;
                case TokenType::AssignDiv:   chunk.emit(OP_DIV_PTR);  break;
                default: return false;
            }

            if (requestType == ValueType::NONE) {
                chunk.emit(OP_POP_STACK);
            }
            return true;


        } else {

            if (!mVarExpr->compile(env,chunk,scope,ValueType::ANY)) return false;
            if (!mRhs->compile(env,chunk,scope,ValueType::ANY)) return false;
            switch(mOp) {
                case TokenType::AssignPlus:  chunk.emit(OP_ADD);  break;
                case TokenType::AssignMinus: chunk.emit(OP_SUB);  break;
                case TokenType::AssignMul:   chunk.emit(OP_MUL);  break;
                case TokenType::AssignDiv:   chunk.emit(OP_DIV);  break;
                default: return false;
            }


            if (!mVarExpr->compileAssignment(env,chunk,scope, requestType)) return false;
            return true;
        }
    }


    // -------------------------------------------------------------------------
    bool TernaryExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mCondition || !mTrueBranch || ! mFalseBranch) {
            PrintASTError(this,"TernaryExpression missing condition or true or false branch!");
            return false;
        }

        if (!mCondition->compile(env,chunk,scope,requestType)) return false;
        chunk.emit(OP_JUMP_IF_FALSE);
        U32 falseJumpMarker = chunk.tell();
        chunk.emit(0); //placeholder

        // if true we use the true branch
        if (!mTrueBranch->compile(env,chunk,scope,requestType)) return false;
        chunk.emit(OP_JUMP);
        U32 endJumpMarker = chunk.tell();
        chunk.emit(0); //placeholder

        chunk.patch(falseJumpMarker, chunk.tell());
        if (!mFalseBranch->compile(env,chunk,scope, ValueType::ANY)) return false;
        chunk.patch(endJumpMarker, chunk.tell());


        return true;
    }
    // -------------------------------------------------------------------------
    bool UnaryMinusExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"Minus Expression without a request!");
            return false;
        }
        if (!mExpr) return false;
        if (!mExpr->compile(env,chunk,scope,requestType)) return false;
        chunk.emit(OP_PUSH_VALUE);
        chunk.emitValue(Value(-1));
        chunk.emit(OP_MUL);

        return true;
    }
    // -------------------------------------------------------------------------
    // FLOW STATEMENTS execute
    // -------------------------------------------------------------------------
    FlowStatus ReturnStatement::execute(Environment& env) {
        // getVariableFrame is gone when it return !!
        VariableFrame* frame = env.getVariableFrame()->getParentFrame();
        if (!frame) frame = env.getVariableFrame();
        if (!frame)  return FlowStatus (FlowSignal::Return);


        FlowStatus flowResult = {FlowSignal::Return};
        if (this->mExpression) {
            flowResult.returnValue = this->mExpression->evaluate(env);
        }
        return flowResult;
    };

    bool ReturnStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (this->mExpression) {
            if (!this->mExpression->compile(env, chunk, scope, ValueType::ANY)) return false;
        } else {
            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(NumberNullValue);
        }

        chunk.emit(OP_RETURN);


        return true;
    }
    // -------------------------------------------------------------------------
    FlowStatus IfStatement::execute(Environment& env){
        Value condVal = this->mCondition->evaluate(env);

        double condNum = condVal.getDouble();
        const double EPSILON = 1e-9;
        bool isTrue = std::abs(condNum) > EPSILON;

        if (isTrue) {
            for (auto& childNode : this->mBody) {
                if (!childNode) continue;
                FlowStatus status = env.execute(childNode.get(), env);
                if (status.signal != FlowSignal::None) return status;
            }
        } else {
            if (mElseBranch != nullptr) {
                FlowStatus status  = env.execute(mElseBranch.get(), env);
                 if (status.signal != FlowSignal::None) return status;

            }
        }
        return  FlowStatus (FlowSignal::None);
    }
    // ----------------
    bool IfStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mCondition.get() ) {
            PrintASTError(this,"If expression is missing condition or then-branch");
            return false;
        }

        if (!mCondition->compile(env, chunk, scope, ValueType::ANY)) return false;

        chunk.emit(OP_JUMP_IF_FALSE);
        U32 jumpToElseOffset = chunk.tell();
        chunk.emit(0);

        for (auto& statement : mBody) {
            if (!statement->compile(env,chunk,scope,ValueType::NONE)) return false;
        }

        if (mElseBranch.get()) {
            chunk.emit(OP_JUMP);
            U32 jumpToEndOffset = chunk.tell();
            chunk.emit(0);

            chunk.patch(jumpToElseOffset, chunk.tell());
            if (!mElseBranch->compile(env, chunk, scope, ValueType::NONE)) return false;
            chunk.patch(jumpToEndOffset, chunk.tell());
        }
        else {
            chunk.patch(jumpToElseOffset, chunk.tell());
        }

        return true;
    }

    // -------------------------------------------------------------------------
    // For Range
    // -------------------------------------------------------------------------
    FlowStatus ForRangeStatement::execute(Environment& env){
        if (!this->mCountExpr ) {
            PrintASTError(this,"Runtime Error: range need a count border!");
            return FlowStatus (FlowSignal::None);
        }
        Value countVal = this->mCountExpr->evaluate(env);
        // Environment loopEnv(&env);

        int32_t count = countVal.getInt();
        if (count < 0 ) {
            PrintASTError(this,"Runtime Error: range border must be >= 0 and is:", std::to_string( count ));
            return FlowStatus (FlowSignal::None);
        }
        // Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        Value* iterValPtr = env.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        for (int i = 0; i < count; i++) {
            *iterValPtr = Value(i);

            for (auto& statement : this->mBody) {
                FlowStatus status = env.execute(statement.get(), env);

                if (status.signal == FlowSignal::Break) {
                    return FlowStatus (FlowSignal::None);
                }
                if (status.signal == FlowSignal::Return) {
                    return status;
                }
                if (status.signal == FlowSignal::Continue) break;
            }
        }
        return  FlowStatus (FlowSignal::None);
    }
    // ------
    bool ForRangeStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!this->mCountExpr ) {
            PrintASTError(this,"forRange need a count border");
            return false;
        }
        // add iter values to registers
        U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);

        // set the limit (count)
        if (!this->mCountExpr->compile(env,chunk,scope,ValueType::Int)) return false;

        chunk.emit(OP_FOR_RANGE);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); //exitIP placeholder
        chunk.emit(iteratorSlot);

        U32 bodyStart = chunk.tell();

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        for (auto& statement : mBody) {
            if (!statement->compile(env,chunk,scope,ValueType::NONE)) return false;
        }
        U32 continueIP = chunk.tell();

        chunk.emit(OP_LOOP_INCREMENT_JUMP);
        chunk.emit(iteratorSlot);
        chunk.emit(bodyStart);
        chunk.emitS32(1); //increment

        U32 loopEndIP = chunk.tell();
        chunk.patch(exitIPPatch, loopEndIP);

        LoopInfo finishedLoop = scope.mLoopStack.back();
        scope.mLoopStack.pop_back();
        for (U32 breakPlaceholder : finishedLoop.breakOffsets) {
            chunk.patch(breakPlaceholder , 2); // we have 2 on the stack
            chunk.patch(breakPlaceholder + 1, loopEndIP);
        }
        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, continueIP);
        }

        return true;
    }
    // -------------------------------------------------------------------------
    FlowStatus ForStatement::execute(Environment& env){
        if (!this->mStartExpr || !this->mEndExpr ) {
            PrintASTError(this,"`for` missing borders!");
            return FlowStatus (FlowSignal::None)  ;
        }
        Value startVal = this->mStartExpr->evaluate(env);
        Value endVal = this->mEndExpr->evaluate(env);

        int start = startVal.getInt();
        int end = endVal.getInt();

        int step = 1;
        if (this->mStepExpr) {
            // step is always positive !!
            Value stepVal = this->mStepExpr->evaluate(env);
            step = std::abs(stepVal.getInt());
        }

        // Environment loopEnv(&env);
        // Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        Value* iterValPtr = env.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        if (start > end ) {
            // for (int i = start; i >= end; --i) {
            for (int i = start; i >= end; i -= step) {
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    // FlowStatus status = env.execute(statement.get(), loopEnv);
                    FlowStatus status = env.execute(statement.get(), env);

                    if (status.signal == FlowSignal::Break) {
                        return FlowStatus (FlowSignal::None);
                    }
                    if (status.signal == FlowSignal::Return) {
                        return status;
                    }
                    if (status.signal == FlowSignal::Continue) break;
                }
            }

        } else {
            for (int i = start; i <= end; i += step) {
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    FlowStatus status = env.execute(statement.get(), env);
                    // FlowStatus status = env.execute(statement.get(), loopEnv);

                    if (status.signal == FlowSignal::Break) {
                        return FlowStatus (FlowSignal::None);
                    }
                    if (status.signal == FlowSignal::Return) {
                        return status;
                    }
                    if (status.signal == FlowSignal::Continue) break;
                }
            }
        }
        return FlowStatus (FlowSignal::None)  ;
    }
    // ---------------------
    bool ForStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mStartExpr || !mEndExpr ) {
            PrintASTError(this,"For-Loop is missing Start, End or Step expression:");
            return false;
        }

        U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);

        // push start, end, increment
        if (!mStartExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
        if (!mEndExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
        if (mStepExpr){

           if (!mStepExpr->compile(env, chunk, scope, ValueType::ANY)) return false;
        } else {
            chunk.emit(OP_PUSH_VALUE);
            chunk.emitValue(Value(1));
        }

        chunk.emit(OP_FOR_STEP_INIT);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); // place holder for loop end
        U32 incrementIPPatch = chunk.tell();
        chunk.emit(0); // incrementIPPatch
        chunk.emit(iteratorSlot);

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        U32 bodyStart = chunk.tell();
        for (auto& statement : mBody) {
            if (!statement->compile(env, chunk, scope, ValueType::NONE)) return false;
        }

        U32 continueIP = chunk.tell();

        chunk.emit(OP_LOOP_INCREMENT_JUMP);
        chunk.emit(iteratorSlot);
        chunk.emit(bodyStart);
        U32 incrementCodePosPatch = chunk.tell();
        chunk.emitS32(0); // increment - patched at runtime

        // we patch here the increment position for increment
        chunk.patch(incrementIPPatch, incrementCodePosPatch);

        U32 loopEndIP = chunk.tell();
        chunk.patch(exitIPPatch, loopEndIP);


        LoopInfo finishedLoop = scope.mLoopStack.back();
        scope.mLoopStack.pop_back();

        for (U32 breakPlaceholder : finishedLoop.breakOffsets) {
            chunk.patch(breakPlaceholder , 2); // we have 2 on the stack
            chunk.patch(breakPlaceholder + 1 , loopEndIP);
        }
        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, continueIP);
        }

        return true;
    }
    // -------------------------------------------------------------------------
    FlowStatus WhileStatement::execute(Environment& env){
        // Environment loopEnv(&env);

        auto checkCondition = [&]() -> bool {
            Value condVal = this->mCondition->evaluate(env);
            return (condVal.getInt() != 0);
        };

        while (checkCondition()) {
            for (auto& statement : this->mBody) {
                FlowStatus status = env.execute(statement.get(), env);

                if (status.signal == FlowSignal::Break) {
                    return FlowStatus (FlowSignal::None);
                }
                if (status.signal == FlowSignal::Return) {
                    return status;
                }
                if (status.signal == FlowSignal::Continue) break;

            }
        }
        return FlowStatus (FlowSignal::None)  ;
    }

    bool WhileStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mCondition.get()) {
            PrintASTError(this,"While loop is missing condition:");
            return false;
        }
        U32 loopStartIP = chunk.tell();
        if (!mCondition->compile(env, chunk, scope, ValueType::ANY)) return false;
        chunk.emit(OP_JUMP_IF_FALSE);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); // placeholder

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        for (auto& statement : mBody) {
            if (!statement->compile(env, chunk, scope, ValueType::NONE)) return false;
        }

        LoopInfo finishedLoop = scope.mLoopStack.back();
        scope.mLoopStack.pop_back();

        chunk.emit(OP_JUMP);
        chunk.emit(loopStartIP);

        U32 loopEndIP = chunk.tell();

        chunk.patch(exitIPPatch, loopEndIP);

        for (U32 breakPlaceholder : finishedLoop.breakOffsets) {
            chunk.patch(breakPlaceholder, 0); // we have no extras on the stack
            chunk.patch(breakPlaceholder + 1, loopEndIP);
        }

        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, loopStartIP);
        }

        return true;
    }


    // -------------------------------------------------------------------------
    FlowStatus ForEachStatement::execute(Environment& env){
        if (!this->mVarExpr ) {
            PrintASTError(this,"Runtime Error: foreach need a list expression!");
            return FlowStatus (FlowSignal::None)  ;
        }

        Value varValue = this->mVarExpr->evaluate(env);

        if (!varValue.isPointer()) {
            PrintASTError(this,"Runtime Error: foreach need a object!");
            return FlowStatus (FlowSignal::None)  ;
        }
        ValueObject* obj = varValue.asPointerObject();
        size_t count = obj->onGetArraySize();
        if (count == 0) {
            return FlowStatus (FlowSignal::None)  ;
        }

        // // Environment loopEnv(&env);

        if (count < 0 ) {
            PrintASTError(this,"Runtime Error: range border must be >= 0 and is:", std::to_string( count ));
            return FlowStatus (FlowSignal::None)  ;
        }

        // Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        Value* iterValPtr = env.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        for (size_t itr = 0; itr < count; itr++) {
            Value* curValue = obj->onGetArrayIndexPtr(itr);
            *iterValPtr = *curValue;

            for (auto& statement : this->mBody) {
                // FlowStatus status = env.execute(statement.get(), loopEnv);
                FlowStatus status = env.execute(statement.get(), env);

                if (status.signal == FlowSignal::Break) {
                    if (iterValPtr) *iterValPtr = NumberNullValue;
                    return FlowStatus (FlowSignal::None);
                }
                if (status.signal == FlowSignal::Return) {
                    if (iterValPtr) *iterValPtr = NumberNullValue;
                    return status;
                }
                if (status.signal == FlowSignal::Continue) break;

            }
        }
        // cleanup
        if (iterValPtr) *iterValPtr = NumberNullValue;

        return  FlowStatus (FlowSignal::None)  ;
    }
    // ---------------------
    bool ForEachStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!this->mVarExpr ) {
            PrintASTError(this,"forEach need a list expression!");
            return false;
        }
        // add iter values to registers
        U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);

        // compile object var expression
        if (!this->mVarExpr->compile(env,chunk,scope,ValueType::Int)) return false;

        chunk.emit(OP_FOR_EACH_INIT);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); //exitIP placeholder
        chunk.emit(iteratorSlot);

        U32 bodyStart = chunk.tell();

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        for (auto& statement : mBody) {
            if (!statement->compile(env,chunk,scope,ValueType::NONE)) return false;
        }
        U32 continueIP = chunk.tell();

        chunk.emit(OP_FOR_EACH_INCREMENT_JUMP);
        chunk.emit(iteratorSlot);
        chunk.emit(bodyStart);

        U32 loopEndIP = chunk.tell();
        chunk.patch(exitIPPatch, loopEndIP);

        LoopInfo finishedLoop = scope.mLoopStack.back();
        scope.mLoopStack.pop_back();
        for (U32 breakPlaceholder : finishedLoop.breakOffsets) {
            chunk.patch(breakPlaceholder , 3); // we have 3 on the stack
            chunk.patch(breakPlaceholder + 1, loopEndIP);
        }
        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, continueIP);
        }

        return true;


    }
    // -------------------------------------------------------------------------
    FlowStatus BlockStatement::execute(Environment& env){
        for (auto& statement : this->mBody) {
            if (!statement) continue;
            FlowStatus status = env.execute(statement.get(), env);
            if (status.signal != FlowSignal::None) return status;
        }
        return FlowStatus (FlowSignal::None) ;
    }

    bool BlockStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        // we fetch the last RESULT
        for (auto& statement : mBody) {
            if (!statement) continue;
            if (!statement->compile(env, chunk, scope, requestType)) return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    Value KeyValueExpression::evaluate(Environment& env) {
        if (!mKeyExpr || !mValueExpr) {
            PrintASTError(this,"KeyValueExpression error key or value missing");
            return NumberNullValue;
        }
        Value keyVal = mKeyExpr->evaluate(env);
        if (!keyVal.isString())  {
            PrintASTError(this,"KeyValueExpression value must be a string,.");
            return NumberNullValue;
        }
        auto obj = new KeyValueObject(keyVal.getStringRef(), mValueExpr->evaluate(env));
        return Value(obj);
    }
    // --------------------
    bool KeyValueExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mKeyExpr || !mValueExpr) {
            PrintASTError(this,"Compile: KeyValueExpression error key or value missing");
            return false;
        }
        if (!mKeyExpr->compile(env,chunk,scope, ValueType::ANY)) return false;
        if (!mValueExpr->compile(env,chunk,scope, ValueType::ANY)) return false;

        chunk.emit(OP_CREATE_KEYVALUE);

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }
        return true;
    }
    // -------------------------------------------------------------------------
    // Include
    // -------------------------------------------------------------------------
    FlowStatus IncludeStatement::execute(DreiZehn::Environment& env){
        if (!mNoExecCondition || !mBody) {
            // default is true so i exit here !
            return {FlowSignal::None};
        }

        Value noExecValue = Value(false);
        noExecValue = mNoExecCondition->evaluate(env);
        if (noExecValue.getBool()) return {FlowSignal::None};
        return mBody->execute(env);
    }
    bool IncludeStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mNoExecCondition || !mBody) {
            // nothing to do here default is true!
            return true;
        }
        // compile condition
        if (!mNoExecCondition->compile(env,chunk,scope,ValueType::Int)) return false;
        // jump if true
        chunk.emit(OP_JUMP_IF_TRUE);
        U32 jmpOverMarker = chunk.tell();
        chunk.emit(0);
        // the body
        if (!mBody->compile(env,chunk,scope,requestType)) return false;
        // path the jump
        chunk.patch(jmpOverMarker, chunk.tell());

        // if (requestType == ValueType::NONE) chunk.emit(OP_POP_STACK);

        return true;
    }
    // -------------------------------------------------------------------------
    // Break
    // -------------------------------------------------------------------------
    bool BreakStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (scope.mLoopStack.empty()) {
            PrintASTError(this,"Error: 'break' outside of loop context!");
            return false;
        }
        chunk.emit(OP_JUMP_CLEAN_STACK);
        U32 breakPlaceholder = chunk.tell();
        chunk.emit(0); // placeholder clean stack count
        chunk.emit(0); // placeholder for jump ip
        scope.mLoopStack.back().breakOffsets.push_back(breakPlaceholder);
        return true;
    }
    // -------------------------------------------------------------------------
    // Continue
    // -------------------------------------------------------------------------
    bool ContinueStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (scope.mLoopStack.empty()) {
            PrintASTError(this,"Error: 'continue' outside of loop context!");
            return false;
        }

        chunk.emit(OP_JUMP);
        U32 continuePlaceholder = chunk.tell();
        chunk.emit(0); // placeholder

        scope.mLoopStack.back().continueOffsets.push_back(continuePlaceholder);

        return true;
    }
    // -------------------------------------------------------------------------
    // StringConcatExpression
    // -------------------------------------------------------------------------
    Value StringConcatExpression::evaluate(Environment& env) {
        if (!mLeft || !mRight) [[unlikely]] {
            PrintASTError(this,"String Concat (Dot Dot ) but left or right is missing");
            return NumberNullValue;
        }
        Value left = mLeft->evaluate(env);
        Value right = mRight->evaluate(env);

        return Value::handleStringAdd(left,right);

    }
    // ------------------------
    bool StringConcatExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (requestType == ValueType::NONE) {
            PrintASTError(this,"Concat but no request for it.");
            return false;
        }

        if (!mLeft.get() || !mRight.get()) {
            PrintASTError(this,"left or right is missing");
            return false;
        }

        if (!this->mLeft->compile(env,chunk, scope, requestType)) return false;
        if (!this->mRight->compile(env,chunk, scope, requestType)) return false;

        chunk.emit(OP_STR_CONCAT);
        return true;
    }
    // -------------------------------------------------------------------------
    // StringConcatAssignExpression
    // -------------------------------------------------------------------------
    Value StringConcatAssignExpression::evaluate(Environment& env) {
        if (!mLeft) [[unlikely]] {
            PrintASTError(this,"String Concat  but variable is missing");
            return NumberNullValue;
        }
        Value* valuePtr = mLeft->evaluatePtr(env);

        if (!valuePtr || !mRight) [[unlikely]] {
            PrintASTError(this,"String Concat (Dot Dot ) but left or right is missing");
            return NumberNullValue;
        }
        Value right = mRight->evaluate(env);

        *valuePtr =Value::handleStringAdd(*valuePtr,right);
        return *valuePtr;

    }
    // ------------------------------
    bool StringConcatAssignExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mLeft) [[unlikely]] {
            PrintASTError(this,"String Concat  but variable is missing");
            return false;
        }


        if (!mLeft->compile(env,chunk,scope,ValueType::ANY)) return false;
        if (!mRight->compile(env,chunk,scope,ValueType::ANY)) return false;
        chunk.emit(OP_STR_CONCAT);


        if (!mLeft->compileAssignment(env,chunk,scope, requestType)) return false;
        return true;

    }
}
