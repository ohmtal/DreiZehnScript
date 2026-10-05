//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Ast for NodeRunner (AST-Interpreter)- i use different code:
// - NodeRunner => evaluate
// - ByteCode => compile
//-----------------------------------------------------------------------------
#include <cmath>
#include "Environment.h"
#include "core/FunctionMap.h"
#include <string.h>


namespace DreiZehn {
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
        return env.getVariableFrame()->getVariable(mVariableNameSymbolId);
    }

    Value* VariableExpression::evaluatePtr(Environment& env) {
        return env.getVariableFrame()->getVariablePtr(mVariableNameSymbolId);
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
        // Value varValue = env.getVariableFrame()->getVariable(mVariableNameSymbolId);
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
    }
    // -------------------------------------------------------------------------
    Value* ObjectFieldExpression::evaluatePtr(Environment& env) {
        Value varValue = mVarExpr->evaluate(env);
        // Value varValue = env.getVariableFrame()->getVariable(mVariableNameSymbolId);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n",varValue.toString().c_str());
            return nullptr;
        }

        ValueObject* obj = varValue.asPointerObject();
        Value* valPtr= obj->onGetFieldPtr(mFieldSymbolId);
        if (!valPtr) {
            Tools::errorf("RunTime Error: Object %s have no field named: %s\n",
                        varValue.toString().c_str(),
                        SymbolTable::getName(mFieldSymbolId).c_str()
            );
        }
        return valPtr;
    }
    // -------------------------------------------------------------------------
    // MethodExpression
    // -------------------------------------------------------------------------
    Value MethodExpression::evaluate(Environment& env) {

        // Value objectPointer = env.getVariableFrame()->getVariable(mPointerNameSymbolId);
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
    Value CallExpression::evaluate(Environment& env) {
        FunctionMap::CallBack* cb = nullptr;

        cb = FunctionMap::GetCFunction(mFuncSymbolId);
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

        const FunctionMap::ScriptFunction* sf = FunctionMap::GetScriptFunction(mFuncSymbolId);

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
                FlowSignal sig = env.execute(statement.get(), localEnv);

                if (sig == FlowSignal::Return) {
                    Value retVal = localEnv.getVariableFrame()->getVariable(SymbolTable::insert("__return_value__"));
                    return retVal;
                }
            }

            return functionResult;
        }

        Tools::errorf("Unknown command: %s\n", SymbolTable::getName(mFuncSymbolId).c_str());
        return Value();

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
                else  return Value(std::string(lVal.toString() + rVal.toString()));
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

        // if (lVal.isDouble() || rVal.isDouble()) {
        //     switch (mOp) {
        //         case TokenType::Plus:  return Value(lVal.getDouble() + rVal.getDouble());
        //         case TokenType::Minus: return Value(lVal.getDouble() - rVal.getDouble());
        //         case TokenType::Mul:   return Value (lVal.getDouble() * rVal.getDouble());
        //         case TokenType::Div: {
        //             double r = rVal.getDouble();
        //             if (r == 0) {
        //                 Tools::errorf("Runtime Error Division by 0! ( %d / %d )\n", lVal.getDouble(), rVal.getDouble());
        //                 return Value(0);
        //             }
        //              return Value (lVal.getDouble()  / r);
        //         }
        //         default: return Value(); // should not reached!
        //     }
        // } else  if (lVal.isInt() && rVal.isInt()) {
        //     switch (mOp) {
        //         case TokenType::Plus:  return Value(lVal.asFastInt() + rVal.asFastInt());
        //         case TokenType::Minus: return Value(lVal.asFastInt() - rVal.asFastInt());
        //         case TokenType::Mul:   {
        //             double d = lVal.getDouble()  * rVal.getDouble();
        //             if (d >= INT32_MIN && d <= INT32_MAX) {
        //                 return Value(static_cast<uint32_t>(d));
        //             }
        //             return d;
        //         }
        //         case TokenType::Div: {
        //             if (rVal.asFastInt() == 0) {
        //                 Tools::errorf("Runtime Error Division by 0! ( %d / %d )\n", lVal.asFastInt(), rVal.asFastInt());
        //                 return Value(0);
        //             }
        //             double l = lVal.getDouble();
        //             double r = rVal.getDouble();
        //             double res = l / r;
        //
        //             return Value(static_cast<int32_t>(res));
        //         }
        //         default: return Value(); // should not reached!
        //     }
        // } else if (lVal.isStringId() || rVal.isStringId()) {
        //     switch (mOp) {
        //         case TokenType::Plus:  return Value(std::string(lVal.toString() + rVal.toString()));
        //         default: return Value(); // return empty invalid operation
        //     }
        // } else {
        //     switch (mOp) {
        //         case TokenType::Plus:  return Value(lVal.getDouble() + rVal.getDouble());
        //         case TokenType::Minus: return Value(lVal.getDouble() - rVal.getDouble());
        //         case TokenType::Mul:   return Value(lVal.getDouble() * rVal.getDouble());
        //         case TokenType::Div:   {
        //             double r = rVal.getDouble();
        //             if (r == 0.0) {
        //                 Tools::PrintRuntimeError("Division by 0.0!\n");
        //                 return Value(0.0);
        //             }
        //
        //             return Value(lVal.getDouble() / r);
        //         }
        //         default: return Value(); // should not reached!
        //     }
        // }


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
            double lCurDouble = valPtr->asDouble();
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
        Value rVal = mRight->evaluate(env);
        return Value(!rVal.getInt());
    }
    // -------------------------------------------------------------------------
    // STATEMENTS execute
    // -------------------------------------------------------------------------
    Value AssignStatement::evaluate(Environment& env) {

        Value * valPtr = mVarExpr->evaluatePtr(env);

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

        if ( !valuePtr  ) {
            Tools::PrintParseError("AssignOP: variable is missing or invalid");
            return Value(0);
        }
        Value rightHand = this->mRhs->evaluate(env);


        if (valuePtr->isDouble() && rightHand.isDouble()) {
            double doubleval = valuePtr->asFastDouble();
            switch(this->mOp) {
                case TokenType::AssignPlus:  doubleval += rightHand.asFastDouble(); break;
                case TokenType::AssignMinus: doubleval -= rightHand.asFastDouble(); break;
                case TokenType::AssignMul:   doubleval *= rightHand.asFastDouble(); break;
                case TokenType::AssignDiv:  rightHand.asFastDouble() != 0.0 ? doubleval /= rightHand.asFastDouble() : doubleval = 0; break;
                default: return Value(0);
            }
            *valuePtr = Value(doubleval);
            return *valuePtr;

        } else if (valuePtr->isInt() && rightHand.isInt()) {
            int32_t intval = valuePtr->asFastInt();
            switch(this->mOp) {
                case TokenType::AssignPlus:  intval += rightHand.asFastInt(); break;
                case TokenType::AssignMinus: intval -= rightHand.asFastInt(); break;
                case TokenType::AssignMul:   intval *= rightHand.asFastInt(); break;
                case TokenType::AssignDiv:  rightHand.asFastInt() != 0 ? intval /= rightHand.asFastInt() : intval = 0; break;
                default: break;
            }
            *valuePtr = Value(intval);
            return *valuePtr;

        } else if (valuePtr->isStringId() || rightHand.isStringId()) {
            if (this->mOp == TokenType::AssignPlus) *valuePtr = Value (std::string( valuePtr->toString() +  rightHand.toString() ));
            else return Value(0);
            return *valuePtr;
        } else {
            double doubleval = valuePtr->getDouble();
            switch(this->mOp) {
                case TokenType::AssignPlus:  doubleval += rightHand.getDouble(); break;
                case TokenType::AssignMinus: doubleval -= rightHand.getDouble(); break;
                case TokenType::AssignMul:   doubleval *= rightHand.getDouble(); break;
                case TokenType::AssignDiv:  rightHand.getDouble() != 0.0 ? doubleval /= rightHand.getDouble() : doubleval = 0.0; break;
                default: return Value(0);
            }
            *valuePtr = Value(doubleval);
            return *valuePtr;
        }

         return Value(0);
    }

    // -------------------------------------------------------------------------
    // FLOW STATEMENTS execute
    // -------------------------------------------------------------------------
    FlowSignal ReturnStatement::execute(Environment& env) {
        // getVariableFrame is gone when it return !!
        VariableFrame* frame = env.getVariableFrame()->getParentFrame();
        if (!frame) frame = env.getVariableFrame();
        if (!frame)  return FlowSignal::Return;

        if (this->mExpression) {
            Value retVal = this->mExpression->evaluate(env);
            frame->setVariable(SymbolTable::insert("__return_value__"), retVal);
        } else {
            frame->setVariable(SymbolTable::insert("__return_value__"), Value(0));
        }
        return FlowSignal::Return;
    };

    // -------------------------------------------------------------------------
    FlowSignal IfStatement::execute(Environment& env){
        Value condVal = this->mCondition->evaluate(env);

        double condNum = condVal.getDouble();
        const double EPSILON = 1e-9;
        bool isTrue = std::abs(condNum) > EPSILON;

        if (isTrue) {
            for (auto& childNode : this->mBody) {
                if (!childNode) continue;
                FlowSignal sig = env.execute(childNode.get(), env);
                if (sig != FlowSignal::None) return sig;
            }
        } else {
            if (mElseBranch != nullptr) {
                FlowSignal sig = env.execute(mElseBranch.get(), env);
                 if (sig != FlowSignal::None) return sig;

            }
        }
        return  FlowSignal::None;
    }

    // -------------------------------------------------------------------------
    FlowSignal ForRangeStatement::execute(Environment& env){
        if (!this->mCountExpr ) {
            Tools::errorf("Runtime Error: range need a count border!\n");
            return FlowSignal::None;
        }
        Value countVal = this->mCountExpr->evaluate(env);
        Environment loopEnv(&env);

        int32_t count = countVal.getInt();
        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
            return FlowSignal::None;
        }
        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId);
        for (int i = 0; i < count; i++) {
            // loopEnv.getVariableFrame()->setVariable(this->mIteratorVarNameSymbolId, Value(i));
            *iterValPtr = Value(i);

            for (auto& statement : this->mBody) {
                FlowSignal sig = env.execute(statement.get(), loopEnv);

                if (sig == FlowSignal::Break) {
                    return FlowSignal::None;
                }
                if (sig == FlowSignal::Return) {
                    return FlowSignal::Return;
                }
                if (sig == FlowSignal::Continue) break;
            }
        }
        return  FlowSignal::None;
    }

    // -------------------------------------------------------------------------
    FlowSignal ForStatement::execute(Environment& env){
        if (!this->mStartExpr || !this->mEndExpr ) {
            Tools::errorf("Runtime Error: invalid for borders!\n");
            return FlowSignal::None;
        }
        Value startVal = this->mStartExpr->evaluate(env);
        Value endVal = this->mEndExpr->evaluate(env);

        int start = startVal.getInt();
        int end = endVal.getInt();

        Environment loopEnv(&env);
        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId);
        if (start > end ) {
            for (int i = start; i >= end; --i) {
                // loopEnv.getVariableFrame()->setVariable(this->mIteratorVarNameSymbolId, Value(i));
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    FlowSignal sig = env.execute(statement.get(), loopEnv);

                    if (sig == FlowSignal::Break) {
                        return FlowSignal::None;
                    }
                    if (sig == FlowSignal::Return) {
                        return FlowSignal::Return;
                    }
                    if (sig == FlowSignal::Continue) break;
                }
            }

        } else {
            for (int i = start; i <= end; ++i) {
                // loopEnv.getVariableFrame()->setVariable(this->mIteratorVarNameSymbolId, Value(i));
                *iterValPtr = Value(i);

                for (auto& statement : this->mBody) {
                    FlowSignal sig = env.execute(statement.get(), loopEnv);

                    if (sig == FlowSignal::Break) {
                        return FlowSignal::None;
                    }
                    if (sig == FlowSignal::Return) {
                        return FlowSignal::Return;
                    }
                    if (sig == FlowSignal::Continue) break;
                }
            }
        }
        return FlowSignal::None;
    }

    // -------------------------------------------------------------------------
    FlowSignal WhileStatement::execute(Environment& env){
        Environment loopEnv(&env);

        auto checkCondition = [&]() -> bool {
            Value condVal = this->mCondition->evaluate(loopEnv);
            return (condVal.getInt() != 0);
            // return (condVal.isInt() && condVal.asFastInt() != 0) ||
            // (condVal.isDouble() && condVal.asFastDouble() != 0.0);
        };

        while (checkCondition()) {
            for (auto& statement : this->mBody) {
                FlowSignal sig = env.execute(statement.get(), loopEnv);

                if (sig == FlowSignal::Break) return FlowSignal::None;
                if (sig == FlowSignal::Return) return FlowSignal::Return;
                if (sig == FlowSignal::Continue) break;
            }
        }
        return FlowSignal::None;
    }
    // -------------------------------------------------------------------------
    FlowSignal ForEachStatement::execute(Environment& env){
        if (!this->mVarExpr ) {
            Tools::errorf("Runtime Error: foreach need a list expression!\n");
            return FlowSignal::None;
        }

        Value varValue = this->mVarExpr->evaluate(env);

        if (!varValue.isPointer()) {
            Tools::errorf("Runtime Error: foreach need a object!\n");
            return FlowSignal::None;
        }
        ValueObject* obj = varValue.asPointerObject();
        size_t count = obj->onGetArraySize();
        if (count == 0) {
            return FlowSignal::None;
        }

        Environment loopEnv(&env);

        if (count < 0 ) {
            Tools::errorf("Runtime Error: range border must be >= 0 and is %d!\n", count);
            return FlowSignal::None;
        }

        Value* iterValPtr = loopEnv.getVariableFrame()->getVariablePtr(this->mIteratorVarNameSymbolId);
        for (size_t itr = 0; itr < count; itr++) {
            Value* curValue = obj->onGetArrayIndexPtr(itr);
            *iterValPtr = *curValue;
            // loopEnv.getVariableFrame()->setVariable(this->mIteratorVarNameSymbolId, *curValue);

            for (auto& statement : this->mBody) {
                FlowSignal sig = env.execute(statement.get(), loopEnv);

                if (sig == FlowSignal::Break) {
                    return FlowSignal::None;
                }
                if (sig == FlowSignal::Return) {
                    return FlowSignal::Return;
                }
                if (sig == FlowSignal::Continue) break;
            }
        }
        return  FlowSignal::None;
    }
    // -------------------------------------------------------------------------
    FlowSignal BlockStatement::execute(Environment& env){
        for (auto& statement : this->mBody) {
            if (!statement) continue;
            FlowSignal sig = env.execute(statement.get(), env);
            if (sig != FlowSignal::None) return sig;
        }
        return FlowSignal::None;
    }
}
