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


namespace DreiZehn {
    // -------------------------------------------------------------------------
    // i need this for variable lookup not found warning
    // to not show up when it's an assign
    bool AssignActive = false;
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
                mEvaluatedValue =  Value(0);
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
    // -------------------------------------------------------------------------
    // VariableExpression
    // -------------------------------------------------------------------------
    Value VariableExpression::evaluate(Environment& env) {
        return *this->evaluatePtr(env);
        // return env.getVariableFrame()->getVariablePtr(mVariableNameSymbolId);
    }

    Value* VariableExpression::evaluatePtr(Environment& env) {

        uint32_t frameID = gCurrentFrame->getFrameId();
        if (mCacheValue && mCacheFrameID == frameID) {
            return mCacheValue;
        }
        mCacheFrameID = frameID;
        mCacheValue =  gCurrentFrame->getVariablePtr(mVariableNameSymbolId, AssignActive);
        return mCacheValue;
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
    // -------------------------------------------------------------------------
    // ArrayVariableExpression
    // -------------------------------------------------------------------------
    Value ArrayVariableExpression::evaluate(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            Tools::errorf("RunTime Error: Array Field invalid Variable or index");
            return Value(0);
        }
        Value varValue = mVarExpr->evaluate(env);
        Value indexValue = mIndexExpr->evaluate(env);
        if (!indexValue.isNumber()) {
            Tools::errorf("RunTime Error: Array Variable: %s. Only number keys allowed\n",varValue.toString().c_str());
            return Value(0);
        }

        // only works on objects which support onGetArrayIndexPtr
        if (!varValue.isPointer() ) {
            return Value(0);
        }
        Value* valPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.asUInt());
        if (valPtr) return Value(*valPtr);

        return Value(0);
    }

    Value* ArrayVariableExpression::evaluatePtr(Environment& env) {
        if (!mVarExpr || !mIndexExpr ) {
            Tools::errorf("RunTime Error: Array Field invalid Variable or index");
            return nullptr;
        }
        Value varValue = mVarExpr->evaluate(env);
        Value indexValue = mIndexExpr->evaluate(env);
        if (!indexValue.isNumber()) {
            Tools::errorf("RunTime Error: Array Variable: %s. Only integer keys allowed\n",varValue.toString().c_str());
            return nullptr;
        }
        // only works on objects which support onGetArrayIndexPtr
        if (!varValue.isPointer() ) {
            return nullptr;
        }
        Value* valPtr = varValue.asPointerObject()->onGetArrayIndexPtr((size_t)indexValue.asUInt());
        return valPtr;
    }
    // -------------------------------------------------------------------------
    // ObjectFieldExpression
    // -------------------------------------------------------------------------
    Value ObjectFieldExpression::evaluate(Environment& env) {
        Value varValue = mVarExpr->evaluate(env);
        Value returnValue = Value(0);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n",varValue.toString().c_str());
            return returnValue;
        }

        ValueObject* obj = varValue.asPointerObject();
        if (obj->onGetField(mFieldSymbolId, returnValue)) {
            return returnValue;
        }
        Tools::errorf("RunTime Error: Object %s have no field named: %s\n",
                      varValue.toString().c_str(),
                      SymbolTable::getName(mFieldSymbolId).c_str()
                      );
        return returnValue;

        // Value* retValuePtr = this->evaluatePtr(env);
        // if (retValuePtr) return *retValuePtr;
        // return Value(0);

    }
    Value* ObjectFieldExpression::evaluatePtr(Environment& env) {
        Value varValue = mVarExpr->evaluate(env);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n",varValue.toString().c_str());
            return nullptr;
        }

        ValueObject* obj = varValue.asPointerObject();

        // NOTE cool speedup but only works on dynamic fields so => NOT!!!!
        // if (mCachedFieldValue && mCachedObject == obj) {
        //     return mCachedFieldValue;
        // }
        //
        // mCachedFieldValue = nullptr;
        // mCachedObject = nullptr;

        Value* valPtr= obj->onGetFieldPtr(mFieldSymbolId);
        if (!valPtr) {
            Tools::errorf("RunTime Error: Object %s have no field named: %s\n",
                        varValue.toString().c_str(),
                        SymbolTable::getName(mFieldSymbolId).c_str()
            );
        }
        // else {
        //     mCachedFieldValue = valPtr;
        //     mCachedObject = obj;
        // }
        return valPtr;
    }
    // -------------------------------------------------------------------------
    // MethodExpression
    // -------------------------------------------------------------------------
    Value MethodExpression::evaluate(Environment& env) {

        Value objectPointer = mVarExpr->evaluate(env);
        if (!objectPointer.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n", objectPointer.toString().c_str());
            return Value();
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

        Value returnValue = Value(0);

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

        Value returnValue = Value(0);
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
            }
        }
        Value functionResult = Value(0);
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
        // test_fibo (33) 6.488u
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

            Value returnValue = Value(0);
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
                }
            }
            Value functionResult = Value(0);
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
                else  return Value(0);
                break;
            }
            case TokenType::Minus:{
                if (mode == 0) return Value(lVal.getDouble() - rVal.getDouble());
                else if (mode == 1) return Value(lVal.getInt() - rVal.getInt());
                else  return Value(0);
                break;

            }
            case TokenType::Mul:{
                double d = lVal.getDouble() * rVal.getDouble();
                if (mode == 0) return (Value(d));
                else if (mode == 1) return (Value(static_cast<int32_t>(d)));
                else  return Value(0);
                break;

            }

            case TokenType::Div: {
                double r = rVal.getDouble();
                if (r == 0.0) return Value(0);
                double d = lVal.getDouble() / r;
                if (mode == 0) return (Value(d));
                else if (mode == 1) return (Value(static_cast<int32_t>(d)));
                else  return Value(0);
                break;

            }
            default: return Value(); // should not reached!
        }


        return Value();

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
    // -------------------------------------------------------------------------

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
                } else if (lVal.isStringId() || rVal.isStringId()) {
                    std::string l = lVal.isStringId() ? lVal.getStringRef() : lVal.toString();
                    std::string r = rVal.isStringId() ? rVal.getStringRef() : rVal.toString();
                    return strcmp(l.c_str(), r.c_str()) == 0;
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
                }
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(std::abs(l - r) < EPSILON ? 0 : 1);
            }
            case TokenType::Or: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l || r);
            }
            case TokenType::And: {
                int l = lVal.getUInt();
                int r = rVal.getUInt();
                return Value( l && r);
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
                        return Value(0);
                    }
                    return Value(std::fmod(lVal.asFastDouble(),rVal.asFastDouble()));
                } else {
                    int64_t l = lVal.getInt64();
                    int64_t r = rVal.getInt64();
                    if (r == 0) {
                        Tools::PrintRuntimeError("Modulo Division by 0!");
                        return Value(0);
                    }
                    int64_t res = l % r;

                    return Value(static_cast<uint32_t>(res));

                }
            }
            default: break;
        }
        return Value();
    }


    // -------------------------------------------------------------------------

    Value BinarySingleRightOpExpression::evaluate(Environment& env)  {
        if (!mRight) return Value();
        Value rVal = mRight->evaluate(env);
        return Value(!rVal.getInt());
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
            return Value(0);
        }

        // since i set it with pointer i need to check assignment here !!!!!!

        Value& pre = *valPtr;
        Value post = this->mRhs->evaluate(env);
        if (pre.isPointer())  pre.asPointerObject()->setAssigned(false);
        if (post.isPointer()) post.asPointerObject()->setAssigned(true);

        *valPtr = post;

        return *valPtr;

    }
    // -------------------------------------------------------------------------

    Value AssignOPStatement::evaluate(Environment& env) {

        Value * valuePtr = mVarExpr->evaluatePtr(env);

        if ( !valuePtr || !this->mRhs ) {
            Tools::PrintParseError("AssignOP: variable or right is missing or invalid");
            return Value(0);
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
                default: return Value(0);
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
            else return Value(0);
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
                default: return Value(0);
            }
            *valuePtr = Value(doubleval);
            return *valuePtr;
        }

        return Value(0);
    }

    // Value AssignOPStatement::evaluate(Environment& env) {
    //
    //     Value * valuePtr = mVarExpr->evaluatePtr(env);
    //
    //     if ( !valuePtr || !this->mRhs ) {
    //         Tools::PrintParseError("AssignOP: variable or right is missing or invalid");
    //         return Value(0);
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
    //                 if (rh == 0) *valuePtr  = Value(0);
    //                 else *valuePtr = Value(valuePtr->asFastInt() / rh);
    //                 return *valuePtr;
    //             }
    //         }
    //         default: return Value(0);
    //     }
    //
    //     return Value(0);
    // }

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

    // -------------------------------------------------------------------------
    FlowStatus ForRangeStatement::execute(Environment& env){
        if (!this->mCountExpr ) {
            Tools::errorf("Runtime Error: range need a count border!\n");
            return FlowStatus (FlowSignal::None);
        }
        Value countVal = this->mCountExpr->evaluate(env);
        Environment loopEnv(&env);

        int32_t count = countVal.getInt();
        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
            return FlowStatus (FlowSignal::None);
        }
        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        for (int i = 0; i < count; i++) {
            *iterValPtr = Value(i);

            for (auto& statement : this->mBody) {
                FlowStatus status = env.execute(statement.get(), loopEnv);

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

        Environment loopEnv(&env);
        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        if (start > end ) {
            for (int i = start; i >= end; --i) {
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    FlowStatus status = env.execute(statement.get(), loopEnv);

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
            for (int i = start; i <= end; ++i) {
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    FlowStatus status = env.execute(statement.get(), loopEnv);

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

    // -------------------------------------------------------------------------
    FlowStatus WhileStatement::execute(Environment& env){
        Environment loopEnv(&env);

        auto checkCondition = [&]() -> bool {
            Value condVal = this->mCondition->evaluate(loopEnv);
            return (condVal.getInt() != 0);
        };

        while (checkCondition()) {
            for (auto& statement : this->mBody) {
                FlowStatus status = env.execute(statement.get(), loopEnv);

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

        Environment loopEnv(&env);

        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
            return FlowStatus (FlowSignal::None)  ;
        }

        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId, true);
        for (size_t itr = 0; itr < count; itr++) {
            Value* curValue = obj->onGetArrayIndexPtr(itr);
            *iterValPtr = *curValue;

            for (auto& statement : this->mBody) {
                FlowStatus status = env.execute(statement.get(), loopEnv);

                if (status.signal == FlowSignal::Break) {
                    return FlowStatus (FlowSignal::None);
                }
                if (status.signal == FlowSignal::Return) {
                    return status;
                }
                if (status.signal == FlowSignal::Continue) break;

            }
        }
        return  FlowStatus (FlowSignal::None)  ;
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
}
