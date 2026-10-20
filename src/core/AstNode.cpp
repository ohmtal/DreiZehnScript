//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// AST-Interpreter
//-----------------------------------------------------------------------------
#include <cmath>
#include "Environment.h"
#include "core/FunctionMap.h"
#include <string.h>
#include "core/BaseObjects.h"

#ifdef DREIZEHN_BYTECODE
#include "bytecode/VMStructure.h"
#include "bytecode/CompilerScope.h"
#endif

namespace DreiZehn {
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

        uint32_t constIdx = chunk.addConstant(this->evaluate(env));
        chunk.emit(OP_CONST);
        chunk.emit(constIdx);
        return true;
    }
    // -------------------------------------------------------------------------
    // ValueExpression
    // -------------------------------------------------------------------------
    bool ValueExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
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
        if (valPtr) return *valPtr;
        return NumberNullValue;
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

        return nullptr;
    }

    bool VariableExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (mGlobalVariable) {
            chunk.emit(OP_LOAD_VAR);
            chunk.emit(mVariableNameSymbolId);
        } else {
            S32 localSlot = scope.resolve(mVariableNameSymbolId);
            if (localSlot < 0 ) {
                std::string varName = SymbolTable::getName(mVariableNameSymbolId);
                Tools::warnf("Variable not found: %s\n", varName.c_str());
                return true; // do not stop!
            }
            chunk.emit(OP_GET_LOCAL);
            chunk.emit(localSlot);
        }
        return true;
    }

    bool VariableExpression::compileAssignment(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, Expression* rhs) {
        // push right
        if (rhs) {
            rhs->compile(env, chunk, scope, ValueType::ANY);
        }

        // save var
        if (mGlobalVariable) {
            chunk.emit(OP_SAVE_VAR);
            chunk.emit(mVariableNameSymbolId);
        } else {
            U32 localSlot = scope.insert(mVariableNameSymbolId);
            chunk.emit(OP_SET_LOCAL);
            chunk.emit(static_cast<U32>(localSlot));
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
    Value* VariableReferenceExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr) return nullptr;
        return mVarExpr->evaluatePtr(env);
    }

    bool VariableReferenceExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
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
    Value* VariablePointerExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr) return nullptr;

        // TODO TESTME - same shit as i did on Variable cache test ?!
        // if (mCachedPtr) return mCachedPtr;

        Value* mCachedPtr = mVarExpr->evaluatePtr(env);
        if (mCachedPtr->isValuePointer()) {
            return mCachedPtr->asValuePointer();
        }
        mCachedPtr  = nullptr;

        return mCachedPtr;
    }
    bool VariablePointerExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    // ArrayVariableExpression
    // -------------------------------------------------------------------------
    Value ArrayVariableExpression::evaluate(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            Tools::warnf("RunTime Error: Array Field invalid Variable or index\n");
            return NumberNullValue;
        }
        Value varValue = mVarExpr->evaluate(env);
        Value indexValue = mIndexExpr->evaluate(env);
        if (indexValue.isNumber()) {
            size_t index = (size_t)indexValue.getUInt();

            if (varValue.isString()) {
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
            Value* valPtr = varValue.asPointerObject()->onGetArrayByStringPtr(indexValue.getStringRef());
            if (valPtr) return Value(*valPtr);
        }

        return NumberNullValue;
    }

    Value* ArrayVariableExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            Tools::warnf("RunTime Error: Array Field invalid Variable or index\n");
            return nullptr;
        }
        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer() ) {
            return nullptr;
        }

        Value indexValue = mIndexExpr->evaluate(env);
        if (indexValue.isNumber()) {
            return varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.asUInt());
        } else if (indexValue.isString()) {

            return varValue.asPointerObject()->onGetArrayByStringPtr(indexValue.getStringRef());
        }
        // only works on objects which support onGetArrayIndexPtr
        return nullptr;
    }

    bool ArrayVariableExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    // ObjectFieldExpression
    // -------------------------------------------------------------------------
    Value ObjectFieldExpression::evaluate(Environment& env) {

        // ------ TEST ONLY << did not speed up ! >>------
        // Value* ptr = this->evaluatePtr(env);
        // if (ptr) return *ptr;
        // // ... fallback ---->
        // ------ ......... ------
        Value returnValue = NumberNullValue;

        if (!mVarExpr) {
             Tools::errorf("RunTime Error expression is invalid\n");
             return returnValue;

        }

        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n",varValue.toString().c_str());
            return returnValue;
        }

        ValueObject* obj = varValue.asPointerObject();
        if (obj->onGetField(mFieldSymbolId, returnValue)) {
            return returnValue;
        }
        Tools::warnf("RunTime: Object %s have no field named: %s\n",
                      varValue.toString().c_str(),
                      SymbolTable::getName(mFieldSymbolId).c_str()
                      );

        return returnValue;
    }
    Value* ObjectFieldExpression::evaluatePtr(Environment& env) {
        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n",varValue.toString().c_str());
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
            Tools::warnf("RunTime: Object %s have no field named: %s\n",
                        varValue.toString().c_str(),
                        SymbolTable::getName(mFieldSymbolId).c_str()
            );
        }
        else {
            mCachedFieldValue = valPtr;
            mCachedObject = obj;
        }
        return valPtr;
    }

    bool ObjectFieldExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    // MethodExpression
    // -------------------------------------------------------------------------
    Value MethodExpression::evaluate(Environment& env) {

        if (!mVarExpr) {
            Tools::errorf("RunTime Error expression is invalid\n");
            return NumberNullValue;

        }
        Value objectPointer = mVarExpr->evaluate(env);
        if (!objectPointer.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n", objectPointer.toString().c_str());
            return NumberNullValue;
        }
        ValueObject* obj = objectPointer.asPointerObject();
        if (!obj) {
            Tools::errorf("RunTime Error: Invalid Object: %s.\n", objectPointer.toString().c_str());
            return Value();
        }
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : mArguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        Value returnValue = NumberNullValue;

        obj->onMethodCall(mMethodNameSymbolId, evaluatedArgs, returnValue);
        return returnValue;

    }


    Value* MethodExpression::evaluatePtr(Environment& env) {
        Value objectPointer = mVarExpr->evaluate(env);
        if (!objectPointer.isPointer()) {
           Tools::errorf("RunTime Error: Object %s not found.\n", objectPointer.toString().c_str());
            return nullptr;
        }
        ValueObject* obj = objectPointer.asPointerObject();
        if (!obj) {
            Tools::errorf("RunTime Error: Invalid Object: %s.\n", objectPointer.toString().c_str());
            return nullptr;
        }
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : mArguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        return obj->onMethodCallGetAssignPtr(mMethodNameSymbolId, evaluatedArgs);
    }

    bool MethodExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    // CallExpression
    // -------------------------------------------------------------------------
#define FUNC_CACHE // about 200ms faster
#ifdef FUNC_CACHE
    Value CallFunc(Environment& env, const FunctionMap::CallBack* cb
        , std::vector<std::unique_ptr<Expression>>& arguments, uint32_t symid) {
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : arguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        Value returnValue = NumberNullValue;
        bool success = (*cb)(evaluatedArgs, returnValue);

        if (!success) {
            Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(symid).c_str());
        }
        return returnValue;
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

        Tools::errorf("Unknown command: %s\n", SymbolTable::getName(mFuncSymbolId).c_str());
        return Value();

    }
#else
    Value CallExpression::evaluate(Environment& env) {

         FunctionMap::CallBack* cb = FunctionMap::GetCFunction(mFuncSymbolId);
        if (cb) {
            std::vector<Value> evaluatedArgs;
            for (auto& argExpr : arguments) {
                if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
            }

            Value returnValue = NumberNullValue;
            bool success = (*cb)(evaluatedArgs, returnValue);

            if (!success) {
                Tools::errorf("Runtime Error in function: %s\n", SymbolTable::getName(mFuncSymbolId).c_str());
            }
            return returnValue;
        }

        FunctionMap::ScriptFunction* sf = FunctionMap::GetScriptFunction(mFuncSymbolId);
        if (sf) {
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

        Tools::errorf("Unknown command: %s\n", SymbolTable::getName(mFuncSymbolId).c_str());
        return Value();

    }
#endif
bool CallExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {

    FunctionMap::CallBack* cb = FunctionMap::GetCFunction(mFuncSymbolId);
    if (cb) {
        U32 argCount = 0;
        for (auto& argExpr : arguments) {
            if (argExpr) {
                argExpr->compile(env, chunk, scope, ValueType::ANY);
                argCount++;
            }
        }
        chunk.emit(OP_CALL_C_FUNCTION);
        chunk.emitPointer(cb);
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
                argExpr->compile(env, chunk, scope, ValueType::ANY);
                argCount++;
            }
        }

        chunk.emit(OP_CALL_SF_FUNCTION);
        chunk.emit(sf->bytecodeAddress);
        chunk.emit(argCount);

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }
        return true;
    }

    //FIXME THIS IS WRONG !!!
    // FunctionMap::ScriptFunction* sf = FunctionMap::GetScriptFunction(mFuncSymbolId);
    // if (sf) {
    //     auto& func = *(sf);
    //     U32 functionStartIP = chunk.tell();
    //     func.bytecodeAddress = functionStartIP;
    //
    //     CompilerScope functionScope;
    //
    //     for (const auto& paramName : func.parameterNames) {
    //         functionScope.insert(SymbolTable::insert(paramName));
    //     }
    //
    //     for (auto& statement : func.body) {
    //         statement->compile(env, chunk, functionScope, ValueType::NONE);
    //     }
    //
    //     chunk.emit(OP_PUSH_VALUE);
    //     chunk.emitValue(NumberNullValue);
    //     chunk.emit(OP_RETURN);
    //
    //     return true;
    // }


    Tools::errorf("Unknown command: %s\n", SymbolTable::getName(mFuncSymbolId).c_str());
    return false;
}

    // -------------------------------------------------------------------------
    // BinaryOpExpression
    // -------------------------------------------------------------------------
    Value BinaryOpExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("left or right is missing:");
            return Value();
        }
        Value lVal = mLeft->evaluate(env);
        Value rVal = mRight->evaluate(env);

        int mode = 0; //double
        if (lVal.isInt() && rVal.isInt()) mode = 1; //int
        else if (lVal.isStringId() || rVal.isStringId()) mode = 2; //string

        switch (mOp) {
            case TokenType::Plus: {
                if (mode == 0) return Value(lVal.getDouble() + rVal.getDouble());
                else if (mode == 1) return Value(lVal.getInt() + rVal.getInt());
                else  if (mode == 2) return Value(std::string(lVal.toString() + rVal.toString()));
                else  return NumberNullValue;
                break;
            }
            case TokenType::Minus:{
                if (mode == 0) return Value(lVal.getDouble() - rVal.getDouble());
                else if (mode == 1) return Value(lVal.getInt() - rVal.getInt());
                else  return NumberNullValue;
                break;

            }
            case TokenType::Mul:{
                double d = lVal.getDouble() * rVal.getDouble();
                if (mode == 0) return (Value(d));
                else if (mode == 1) return (Value(static_cast<int32_t>(d)));
                else  return NumberNullValue;
                break;

            }

            case TokenType::Div: {
                double r = rVal.getDouble();
                if (r == 0.0) return NumberNullValue;
                double d = lVal.getDouble() / r;
                if (mode == 0) return (Value(d));
                else if (mode == 1) return (Value(static_cast<int32_t>(d)));
                else  return NumberNullValue;
                break;

            }
            default: return Value(); // should not reached!
        }


        return Value();

    }
    // --------
    bool BinaryOpExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("left or right is missing:");
            return false;
        }
        this->mLeft->compile(env,chunk, scope, requestType);
        this->mRight->compile(env,chunk, scope, requestType);

        switch (this->mOp) {
            case TokenType::Plus    : chunk.emit(OP_ADD); break;
            case TokenType::Minus   : chunk.emit(OP_SUB); break;
            case TokenType::Mul     : chunk.emit(OP_MUL); break;
            case TokenType::Div     : chunk.emit(OP_DIV); break;
            default:
                Tools::errorf("COMPILE ERROR BinaryOpExpression : unknown TokenType: %d", (int)this->mOp);
                return false;
        }
        return true;
    }
    // -------------------------------------------------------------------------
    // ++ --
    Value BinaryInlineExpression::evaluate(Environment& env)  {

        Value * valPtr = mExpr->evaluatePtr(env);


        if ( !valPtr  ) {
            Tools::PrintParseError("variable is missing or invalid");
            return Value();
        }

        if (valPtr->isInt() ) {
            int32_t lCurInt = valPtr->asInt();
            switch (mOp) {
                case TokenType::PlusPlus: lCurInt++; break;
                case TokenType::MinusMinus: lCurInt--; break;
                default: break;
            }
            *valPtr = Value(lCurInt);
            return *valPtr;
        } else  { // must be a Double!
            double lCurDouble = valPtr->getDouble();
            switch (mOp) {
                case TokenType::PlusPlus: lCurDouble += 1.0 ; break;
                case TokenType::MinusMinus: lCurDouble -= 1.0 ; break;
                default: break;
            }
            *valPtr = Value(lCurDouble);
            return *valPtr;
        }


        return Value();

    }
    // ---------------------
    bool BinaryInlineExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (!mExpr) return false;

        S32 inc = (mOp == TokenType::PlusPlus) ? 1 : - 1;


        // 1. i need to know if it's a local - this is not save LOL
        if (mExpr->mNodeType == NodeType::VariableExpression) {
            VariableExpression* vExpr = static_cast<VariableExpression*>(mExpr.get());
            if (!vExpr->mGlobalVariable) {
                S32 localSlot = scope.resolve(vExpr->mVariableNameSymbolId);
                if (localSlot < 0 ) {
                    std::string varName = SymbolTable::getName(vExpr->mVariableNameSymbolId);
                    Tools::warnf("Variable not found: %s\n", varName.c_str());
                    return true; // do not stop!
                }

                chunk.emit(OP_INC_LOCAL);
                chunk.emit(localSlot);
                chunk.emitS32(inc);


                return true;
            } else {
                // PUSH VAR
                chunk.emit(OP_LOAD_VAR);
                chunk.emit(vExpr->mVariableNameSymbolId);
                // PUSH INC
                chunk.emit(OP_PUSH_VALUE);
                chunk.emitValue(Value(inc));
                // ADD INC
                chunk.emit(OP_ADD);
                // SAVE VAR
                chunk.emit(OP_SAVE_VAR);
                chunk.emit(vExpr->mVariableNameSymbolId);
                return true;
            }
        }
        //Hybrid HACK fest !!!!
        chunk.emit(OP_INC_PTR);
        chunk.emitPointer(mExpr.get());
        chunk.emitS32(inc);

        return false;
    }
    // -------------------------------------------------------------------------
    Value BinaryAndOrExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("(&& ||) left or right is missing:");
            return Value();
        }

        if (mOp == TokenType::And) {
            Value lVal = mLeft->evaluate(env);
            int l = lVal.getUInt();
            if (l == 0) return Value(0);
            Value rVal = mRight->evaluate(env);
            int r = rVal.getUInt();
            return Value( l && r);
        } else
        if (mOp == TokenType::Or) {
            Value lVal = mLeft->evaluate(env);
            int l = lVal.getUInt();
            if (l == 1) return Value(1);
            Value rVal = mRight->evaluate(env);
            int r = rVal.getUInt();
            return Value( l || r);
        }

        return NumberNullValue;

    }
    bool BinaryAndOrExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){

        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("(&& ||) left or right is missing:");
            return false;
        }

        mLeft->compile(env,chunk,scope, ValueType::ANY);

        if (mOp == TokenType::Or) {
            chunk.emit(OP_JUMP_IF_TRUE_NO_POP);
            U32 jumpOffset = chunk.tell();
            chunk.emit(0); //placeholder
            chunk.emit(OP_POP_STACK);
            mRight->compile(env,chunk,scope, ValueType::ANY);
            chunk.patch(jumpOffset, chunk.tell());

        } else { // TokenType::And
            chunk.emit(OP_JUMP_IF_FALSE_NO_POP);
            U32 jumpOffset = chunk.tell(); // Zeigt direkt auf den kommenden Platzhalter
            chunk.emit(0); // placeholder

            chunk.emit(OP_POP_STACK);
            mRight->compile(env, chunk, scope, ValueType::ANY);

            chunk.patch(jumpOffset, chunk.tell());
        }

        if (requestType == ValueType::NONE)
             chunk.emit(OP_POP_STACK);

        return false;
    }
    // -------------------------------------------------------------------------
    bool BinaryExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("left or right is missing:");
            return false;
        }

        mRight->compile(env, chunk, scope, ValueType::ANY);
        mLeft->compile(env, chunk, scope, ValueType::ANY);

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
                Tools::PrintParseError("Unknown binary operator");
                return false;
        }

        if (requestType == ValueType::NONE) {
            chunk.emit(OP_POP_STACK);
        }

        return true;
    }
    // -------------------
    Value BinaryExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("left or right is missing:");
            return Value();
        }


        Value lVal = mLeft->evaluate(env);
        Value rVal = mRight->evaluate(env);

        switch (mOp) {
            case TokenType::Greater: {
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value((l - r) > EPSILON ? 1 : 0);
            }
            case TokenType::GreaterEqual: {
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(l > (r - EPSILON) ? 1 : 0);
            }
            case TokenType::Less:  {
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value((r - l) > EPSILON ? 1 : 0);
            }
            case TokenType::LowerEqual: {
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(l < (r + EPSILON) ? 1 : 0);
            }
            case TokenType::Equal: {
                if (lVal.isPointer() && rVal.isPointer()) {
                    return Value(lVal.asPointer() == rVal.asPointer() ? 1 : 0);
                } else if (lVal.isStringId() && rVal.isStringId()) {
                    return strcmp(lVal.getStringRef().c_str(), rVal.getStringRef().c_str()) == 0;
                } else if (lVal.isStringId() || rVal.isStringId()) {
                    return NumberNullValue;
                }
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(std::abs(l - r) < EPSILON ? 1 : 0);
            }
            case TokenType::NotEqual: {
                if (lVal.isPointer() && rVal.isPointer()) {
                    return Value(lVal.asPointer() != rVal.asPointer() ? 1 : 0);
                } else if (lVal.isStringId() && rVal.isStringId()) {
                    return strcmp(lVal.getStringRef().c_str(), rVal.getStringRef().c_str()) != 0;
                }  else if (lVal.isStringId() || rVal.isStringId()) {
                    return Value(1);
                }
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(std::abs(l - r) < EPSILON ? 0 : 1);
            }

            case TokenType::BitAnd: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l & r);
            }
            case TokenType::BitOr: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l | r);
            }
            case TokenType::BitXOr: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l ^ r);
            }
            case TokenType::SHL: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l << r);
            }
            case TokenType::SHR: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l >> r);
            }
            case TokenType::Modulo: {

                if (lVal.isDouble() && rVal.isDouble() ) {
                    double r = rVal.asFastDouble();
                    if (r == 0.0) {
                        Tools::PrintRuntimeError("Modulo Division by 0.0!");
                        return NumberNullValue;
                    }
                    return Value(std::fmod(lVal.asFastDouble(),rVal.asFastDouble()));
                } else {
                    int64_t l = lVal.getInt64();
                    int64_t r = rVal.getInt64();
                    if (r == 0) {
                        Tools::PrintRuntimeError("Modulo Division by 0!");
                        return NumberNullValue;
                    }
                    int64_t res = l % r;

                    return Value(static_cast<uint32_t>(res));

                }
            }
            default: break;
        }
        return NumberNullValue;
    }


    // -------------------------------------------------------------------------

    Value BinarySingleRightOpExpression::evaluate(Environment& env)  {
        if (!mRight) return Value();
        Value rVal = mRight->evaluate(env);
        return Value(!rVal.getInt());
    }
    bool BinarySingleRightOpExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }

    // -------------------------------------------------------------------------
    // STATEMENTS execute
    // -------------------------------------------------------------------------
    Value AssignStatement::evaluate(Environment& env) {

        AssignActive = true;
        Value * valPtr = mVarExpr->evaluatePtr(env);
        AssignActive = false;

        if ( !valPtr  ) {
            Tools::PrintParseError("Assign: variable is missing or invalid");
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

        // types must support compileAssignment!
        return mVarExpr->compileAssignment(env, chunk, scope, mRhs.get());

    }

    // -------------------------------------------------------------------------
    Value AssignOPStatement::evaluate(Environment& env) {

        Value * valuePtr = mVarExpr->evaluatePtr(env);

        if ( !valuePtr || !this->mRhs ) {
            Tools::PrintParseError("AssignOP: variable or right is missing or invalid");
            return NumberNullValue;
        }
        Value rightHand = this->mRhs->evaluate(env);


        if (valuePtr->isDouble() && rightHand.isDouble()) {
            double doubleval = valuePtr->asFastDouble();
            switch(this->mOp) {
                case TokenType::AssignPlus:  doubleval += rightHand.asFastDouble(); break;
                case TokenType::AssignMinus: doubleval -= rightHand.asFastDouble(); break;
                case TokenType::AssignMul:   doubleval *= rightHand.asFastDouble(); break;
                case TokenType::AssignDiv:  {
                    double r = rightHand.asFastDouble();
                    r != 0.0 ? doubleval /= r : doubleval = 0; break;
                }
                default: return NumberNullValue;
            }
            *valuePtr = Value(doubleval);
            return *valuePtr;

        } else if (valuePtr->isInt( ) && rightHand.isInt()) {
            int32_t intval = valuePtr->asFastInt();
            switch(this->mOp) {
                case TokenType::AssignPlus:  intval += rightHand.asFastInt(); break;
                case TokenType::AssignMinus: intval -= rightHand.asFastInt(); break;
                case TokenType::AssignMul:   intval *= rightHand.asFastInt(); break;
                case TokenType::AssignDiv:  {
                    int32_t r = rightHand.asFastInt();
                    r != 0 ? intval /= r : intval = 0; break;
                }
                default: break;
            }
            *valuePtr = Value(intval);
            return *valuePtr;

        } else if (valuePtr->isStringId( )) {
            if (this->mOp == TokenType::AssignPlus) *valuePtr = Value (valuePtr->getStringRef() +  rightHand.toString() );
            else return NumberNullValue;
            return *valuePtr;

        } else {
            double doubleval = valuePtr->getDouble();
            switch(this->mOp) {
                case TokenType::AssignPlus:  doubleval += rightHand.getDouble(); break;
                case TokenType::AssignMinus: doubleval -= rightHand.getDouble(); break;
                case TokenType::AssignMul:   doubleval *= rightHand.getDouble(); break;
                case TokenType::AssignDiv: {
                    double r = rightHand.asFastDouble();
                    r != 0.0 ? doubleval /= r : doubleval = 0; break;
                }
                default: return NumberNullValue;
            }
            *valuePtr = Value(doubleval);
            return *valuePtr;
        }

        return NumberNullValue;
    }

    bool AssignOPStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }

    // Value AssignOPStatement::evaluate(Environment& env) {
    //
    //     Value * valuePtr = mVarExpr->evaluatePtr(env);
    //
    //     if ( !valuePtr || !this->mRhs ) {
    //         Tools::PrintParseError("AssignOP: variable or right is missing or invalid");
    //         return NumberNullValue;
    //     }
    //     Value rightHand = this->mRhs->evaluate(env);
    //
    //     int mode = 0;
    //     if (valuePtr->isDouble()) mode = 0; //double
    //     if (valuePtr->isInt() && rightHand.isInt()) mode = 1; //int
    //     else if (valuePtr->isStringId() ) mode = 2; //string
    //
    //     switch(this->mOp) {
    //         case TokenType::AssignPlus: {
    //             if (mode == 0) {
    //                 *valuePtr = Value(valuePtr->asFastDouble() + rightHand.getDouble());
    //                 return *valuePtr;
    //             } else if (mode == 1) {
    //                 *valuePtr = Value(valuePtr->asFastInt() + rightHand.getInt());
    //                 return *valuePtr;
    //             } else {
    //                 *valuePtr = Value (valuePtr->getStringRef() +  rightHand.toString() );
    //                 return *valuePtr;
    //             }
    //         }
    //         case TokenType::AssignMinus: {
    //             if (mode == 0) {
    //                 *valuePtr = Value(valuePtr->asFastDouble() - rightHand.getDouble());
    //                 return *valuePtr;
    //             } else if (mode == 1) {
    //                 *valuePtr = Value(valuePtr->asFastInt() - rightHand.getInt());
    //                 return *valuePtr;
    //             }
    //         }
    //         case TokenType::AssignMul: {
    //             if (mode == 0) {
    //                 *valuePtr = Value(valuePtr->asFastDouble() * rightHand.getDouble());
    //                 return *valuePtr;
    //             } else if (mode == 1) {
    //                 *valuePtr = Value(valuePtr->asFastInt() * rightHand.getInt());
    //                 return *valuePtr;
    //             }
    //         }
    //         case TokenType::AssignDiv:  {
    //             if (mode == 0) {
    //                 double rh = rightHand.getDouble();
    //                 *valuePtr = Value(valuePtr->asFastDouble() / (rh == 0.0 ? EPSILON : rh));
    //                 return *valuePtr;
    //             } else if (mode == 1) {
    //                 int32_t rh = rightHand.getDouble();
    //                 if (rh == 0) *valuePtr  = NumberNullValue;
    //                 else *valuePtr = Value(valuePtr->asFastInt() / rh);
    //                 return *valuePtr;
    //             }
    //         }
    //         default: return NumberNullValue;
    //     }
    //
    //     return NumberNullValue;
    // }

    // -------------------------------------------------------------------------
    bool TernaryExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    bool UnaryMinusExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
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
            this->mExpression->compile(env, chunk, scope, ValueType::ANY);
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
            Tools::PrintParseError("If expression is missing condition or then-branch:");
            return false;
        }

        mCondition->compile(env, chunk, scope, ValueType::ANY);

        chunk.emit(OP_JUMP_IF_FALSE);
        U32 jumpToElseOffset = chunk.tell();
        chunk.emit(0);

        for (auto& statement : mBody) {
            statement->compile(env,chunk,scope,ValueType::NONE);
        }

        if (mElseBranch.get()) {
            chunk.emit(OP_JUMP);
            U32 jumpToEndOffset = chunk.tell();
            chunk.emit(0);

            chunk.patch(jumpToElseOffset, chunk.tell());
            mElseBranch->compile(env, chunk, scope, ValueType::NONE);
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
            Tools::errorf("Runtime Error: range need a count border!\n");
            return FlowStatus (FlowSignal::None);
        }
        Value countVal = this->mCountExpr->evaluate(env);
        // Environment loopEnv(&env);

        int32_t count = countVal.getInt();
        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
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
            Tools::errorf("Runtime Error: range need a count border!\n");
            return false;
        }
        // add iter values to registers
        U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);
        // set the start iter to 0
        chunk.emit(OP_PUSH_VALUE);
        chunk.emitValue(Value(0));
        chunk.emit(OP_SET_LOCAL);
        chunk.emit(iteratorSlot);

        // set the limit (count)
        this->mCountExpr->compile(env,chunk,scope,ValueType::Int);

        chunk.emit(OP_FOR_RANGE);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); //exitIP placeholder
        chunk.emit(iteratorSlot);

        U32 bodyStart = chunk.tell();

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        for (auto& statement : mBody) {
            statement->compile(env,chunk,scope,ValueType::NONE);
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
            chunk.patch(breakPlaceholder, loopEndIP);
        }
        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, continueIP);
        }

        return true;
    }
    //NOTE works but  too slow .. need something like in ElfScript!
    // one billion test : 10.786u
    // // bool ForRangeStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
    // //     if (!this->mCountExpr ) {
    // //         Tools::errorf("Runtime Error: range need a count border!\n");
    // //         return false;
    // //     }
    // //
    // //     // add iter values to registers
    // //     U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);
    // //     U32 endValueTmpSlot = scope.allocateTemporarySlot();
    // //
    // //     // set the start iter to 0
    // //     chunk.emit(OP_PUSH_VALUE);
    // //     chunk.emitValue(Value(0));
    // //     chunk.emit(OP_SET_LOCAL);
    // //     chunk.emit(iteratorSlot);
    // //
    // //     // set the end iter:
    // //     this->mCountExpr->compile(env,chunk,scope,ValueType::Int);
    // //     chunk.emit(OP_SET_LOCAL);
    // //     chunk.emit(endValueTmpSlot);
    // //
    // //
    // //     U32 loopConditionAddr = chunk.tell();
    // //
    // //     // ITER
    // //     chunk.emit(OP_GET_LOCAL);
    // //     chunk.emit(iteratorSlot);
    // //     chunk.emit(OP_GET_LOCAL);
    // //     chunk.emit(endValueTmpSlot);
    // //     chunk.emit(OP_LESS);
    // //
    // //     chunk.emit(OP_JUMP_IF_FALSE);
    // //     U32 exitJumpPlaceholder = chunk.tell();
    // //     chunk.emit(0); // space for patch
    // //
    // //     for (auto& statement : mBody) {
    // //         //NOTE not sure about the ValueType
    // //         statement->compile(env,chunk,scope,ValueType::ANY);
    // //     }
    // //
    // //     chunk.emit(OP_INC_LOCAL);
    // //     chunk.emit(iteratorSlot);
    // //     chunk.emit(1);
    // //
    // //     chunk.emit(OP_JUMP);
    // //     chunk.emit(loopConditionAddr);
    // //
    // //     // patch jmp
    // //     U32 loopEndAddr = chunk.tell();
    // //     chunk.patch(exitJumpPlaceholder, loopEndAddr);
    // //
    // //     scope.freeTemporarySlot();
    // //
    // //
    // //     return true;
    // // }
    // -------------------------------------------------------------------------
    FlowStatus ForStatement::execute(Environment& env){
        if (!this->mStartExpr || !this->mEndExpr ) {
            Tools::errorf("Runtime Error: invalid for borders!\n");
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
            Tools::PrintParseError("For-Loop is missing Start, End or Step expression:");
            return false;
        }

        U32 iteratorSlot = scope.insert(mIteratorVarNameSymbolId);

        // push start, end, increment
        mStartExpr->compile(env, chunk, scope, ValueType::ANY);
        mEndExpr->compile(env, chunk, scope, ValueType::ANY);
        if (mStepExpr)
            mStepExpr->compile(env, chunk, scope, ValueType::ANY);
        else {
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
            statement->compile(env, chunk, scope, ValueType::NONE);
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
            chunk.patch(breakPlaceholder, loopEndIP);
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
            Tools::PrintParseError("While loop is missing condition:");
            return false;
        }
        U32 loopStartIP = chunk.tell();
        mCondition->compile(env, chunk, scope, ValueType::ANY);
        chunk.emit(OP_JUMP_IF_FALSE);
        U32 exitIPPatch = chunk.tell();
        chunk.emit(0); // placeholder

        LoopInfo loop;
        scope.mLoopStack.push_back(loop);

        for (auto& statement : mBody) {
            statement->compile(env, chunk, scope, ValueType::NONE);
        }

        LoopInfo finishedLoop = scope.mLoopStack.back();
        scope.mLoopStack.pop_back();

        chunk.emit(OP_JUMP);
        chunk.emit(loopStartIP);

        U32 loopEndIP = chunk.tell();

        chunk.patch(exitIPPatch, loopEndIP);

        for (U32 breakPlaceholder : finishedLoop.breakOffsets) {
            chunk.patch(breakPlaceholder, loopEndIP);
        }

        for (U32 contPlaceholder : finishedLoop.continueOffsets) {
            chunk.patch(contPlaceholder, loopStartIP);
        }

        return true;
    }


    // -------------------------------------------------------------------------
    FlowStatus ForEachStatement::execute(Environment& env){
        if (!this->mVarExpr ) {
            Tools::errorf("Runtime Error: foreach need a list expression!\n");
            return FlowStatus (FlowSignal::None)  ;
        }

        Value varValue = this->mVarExpr->evaluate(env);

        if (!varValue.isPointer()) {
            Tools::errorf("Runtime Error: foreach need a object!\n");
            return FlowStatus (FlowSignal::None)  ;
        }
        ValueObject* obj = varValue.asPointerObject();
        size_t count = obj->onGetArraySize();
        if (count == 0) {
            return FlowStatus (FlowSignal::None)  ;
        }

        // // Environment loopEnv(&env);

        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
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

    bool ForEachStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
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
            statement->compile(env, chunk, scope, requestType);
        }
        return true;
    }
    // -------------------------------------------------------------------------
    Value KeyValueExpression::evaluate(Environment& env) {
        if (!mKeyExpr || !mValueExpr) {
            Tools::PrintRuntimeError("KeyValueExpression error key or value missing");
            return NumberNullValue;
        }
        Value keyVal = mKeyExpr->evaluate(env);
        if (!keyVal.isString())  {
            Tools::PrintRuntimeError("KeyValueExpression value must be a string,.");
            return NumberNullValue;
        }
        auto obj = new KeyValueObject(keyVal.getStringRef(), mValueExpr->evaluate(env));
        return Value(obj);
    }

    bool KeyValueExpression::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType){
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    bool IncludeStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        //FIXME
        return false;
    }
    // -------------------------------------------------------------------------
    bool BreakStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (scope.mLoopStack.empty()) {
            Tools::PrintParseError("Error: 'break' outside of loop context!");
            return false;
        }
        chunk.emit(OP_JUMP);
        U32 breakPlaceholder = chunk.tell();
        chunk.emit(0); // placeholder for
        scope.mLoopStack.back().breakOffsets.push_back(breakPlaceholder);
        return true;
    }
    // -------------------------------------------------------------------------
    bool ContinueStatement::compile(Environment& env, BytecodeChunk& chunk, CompilerScope& scope, ValueType requestType) {
        if (scope.mLoopStack.empty()) {
            Tools::PrintParseError("Error: 'continue' outside of loop context!");
            return false;
        }

        chunk.emit(OP_JUMP);
        U32 continuePlaceholder = chunk.tell();
        chunk.emit(0); // placeholder

        scope.mLoopStack.back().continueOffsets.push_back(continuePlaceholder);

        return true;
    }
    // -------------------------------------------------------------------------
}
