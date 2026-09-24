//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Ast Noded for NodeRunner - i use different code:
// - NodeRunner => evaluate
// - ByteCode => compile
//-----------------------------------------------------------------------------
#include "Environment.h"
#include "core/FunctionMap.h"


namespace DreiZehn {
    // -------------------------------------------------------------------------
    Value LiteralExpression::evaluate(Environment& env) {
        if (mType == TokenType::Number) {
            char* endptr = nullptr;
            bool  haveDot = mRawValue.find('.') != std::string::npos;
            double resDouble = std::strtod(mRawValue.c_str(), &endptr);
            if (mRawValue.empty() || *endptr != '\0') {
                return Value(0);
            } else {
                if (haveDot) return Value(resDouble);
                else return Value((int32_t)resDouble);
            }
        }

        if (mType == TokenType::StringLiteral) {
            auto* strObj = new StringValueObject(mRawValue);

            env.getVariableFrame()->addToGarbageCollection(strObj);

            return Value(strObj);
        }
        return Value(); // Fallback
    }
    // -------------------------------------------------------------------------
    Value VariableExpression::evaluate(Environment& env) {
        // return env.getVariable(SymbolTable::insert(mName));
        return env.getVariableFrame()->getVariable(mVariableNameSymbolId);

    }
    // -------------------------------------------------------------------------
    Value ObjectFieldExpression::evaluate(Environment& env) {
        Value varValue = env.getVariableFrame()->getVariable(mVariableNameSymbolId);
        Value returnValue = Value(0);
        if (!varValue.isPointer()) {
            Tools::errorf("RunTime Error: Object %s not found.\n", SymbolTable::getName(mVariableNameSymbolId).c_str());
            return returnValue;
        }

        ValueObject* obj = dynamic_cast<ValueObject*>(varValue.asPointerObject());
        if (obj->onGetField(mFieldSymbolId, returnValue)) {
            return returnValue;
        }
        Tools::errorf("RunTime Error: Object %s have no field named: %s\n",
                      SymbolTable::getName(mVariableNameSymbolId).c_str(),
                      SymbolTable::getName(mFieldSymbolId).c_str()
                      );
        return returnValue;
    }
    // -------------------------------------------------------------------------
    Value MethodExpression::evaluate(Environment& env) {
        Value objectPointer = env.getVariableFrame()->getVariable(mPointerNameSymbolId);
        if (!objectPointer.isPointer()) {
            // Tools::errorf("RunTime Error: Object %s not found.\n", mPointerName.c_str());
            Tools::errorf("RunTime Error: Object %s not found.\n", SymbolTable::getName(mPointerNameSymbolId).c_str());
            return Value();
        }
        ValueObject* obj = dynamic_cast<ValueObject*>(objectPointer.asPointerObject());
        if (!obj) {
            Tools::errorf("RunTime Error: Invalid Object: %s.\n", SymbolTable::getName(mPointerNameSymbolId).c_str());
            return Value();
        }
        std::vector<Value> evaluatedArgs;
        for (auto& argExpr : mArguments) {
            if (argExpr) evaluatedArgs.push_back(argExpr->evaluate(env));
        }

        Value returnValue = Value(0);

        if (!obj->onMethodCall(mMethodNameSymbolId, evaluatedArgs, returnValue)) {
            // let the method handle this.
            // Tools::errorf("Runtime Error in method call: %s -> %s\n",
            //               SymbolTable::getName(mPointerNameSymbolId).c_str(),
            //               SymbolTable::getName(mMethodNameSymbolId).c_str()
            // );
        }
        return returnValue;

    }
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
                    localEnv.getVariableFrame()->setVariable(SymbolTable::insert(func.parameterNames[i]), evaluatedArg);
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

    Value BinaryOpExpression::evaluate(Environment& env)  {
        if (!mLeft.get() || !mRight.get()) {
            Tools::PrintParseError("left or right is missing:");
            return Value();
        }
        Value lVal = mLeft->evaluate(env);
        Value rVal = mRight->evaluate(env);

        if (lVal.isInt() && rVal.isInt()) {
            if (mOp == TokenType::Plus) return Value(lVal.asInt() + rVal.asInt());
            if (mOp == TokenType::Minus) return Value(lVal.asInt() - rVal.asInt());
            if (mOp == TokenType::Mul) return Value(lVal.asInt() * rVal.asInt());
            if (mOp == TokenType::Div) return Value(lVal.asInt() / rVal.asInt());
        }
        double lNum = lVal.getDouble();
        double rNum = rVal.getDouble();

        if (mOp == TokenType::Plus) return Value(lNum + rNum);
        if (mOp == TokenType::Minus) return Value(lNum - rNum);
        if (mOp == TokenType::Mul) return Value(lNum * rNum);
        if (mOp == TokenType::Div) return Value(lNum / rNum);

        return Value();

    }
    // -------------------------------------------------------------------------
    // ++ --
    Value BinaryInlineExpression::evaluate(Environment& env)  {
        if ( mVarNameSymbolId == 0 ) {
            Tools::PrintParseError("variable is missing:");
            return Value();
        }
        Value* valPtr = env.getVariableFrame()->getVariablePtr(mVarNameSymbolId);
        if (!valPtr || valPtr->isPointer()) {
            Tools::errorf("Invalid Pointer operation: %s\n",tokenTypeToString(mOp));
            return Value();
        }
        // NOTE: should i cast it to int?!

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
                }
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(std::abs(l - r) < EPSILON ? 1 : 0);
            }
            case TokenType::NotEqual: {
                if (lVal.isPointer() && rVal.isPointer()) {
                    return Value(lVal.asPointer() != rVal.asPointer() ? 1 : 0);
                }
                double l = lVal.getDouble();
                double r = rVal.getDouble();
                return Value(std::abs(l - r) < EPSILON ? 0 : 1);
            }
            case TokenType::Or: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l || r);
            }
            case TokenType::And: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l && r);
            }
            case TokenType::BitAnd: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l & r);
            }
            case TokenType::BitOr: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l | r);
            }
            case TokenType::SHL: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l << r);
            }
            case TokenType::SHR: {
                int l = lVal.getInt();
                int r = rVal.getInt();
                return Value( l >> r);
            }
            default: break;
        }
        return Value();
    }


    // -------------------------------------------------------------------------

}
