//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Enviroment VM
//-----------------------------------------------------------------------------
#pragma once
//-----------------------------------------------------------------------------
#ifdef DREIZEHN_DEBUG
// #define DEBUG_TRACE_EXECUTION
#endif
//-----------------------------------------------------------------------------
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

#include <unordered_map>
#include <iostream>
#include <functional>
#include <cassert>

#include "core/AstNode.h"
#include "core/FunctionMap.h"
#include "core/Value.h"
#include "core/VariableFrame.h"

#include "Globals.h"

#include "toolbox/SymbolTable.h"
#include "toolbox/Tools.h"
#include "bytecode/VM.h"


namespace DreiZehn {


class Environment {
private:

    // -- locals Register --
    std::unordered_map<uint32_t, size_t> mRootLocalsLookUp;
    std::vector<size_t> mLocalVarCount;
    U32 mLocalsFrameId = 0;
    size_t mLocalOffset = 0;
    size_t mLocalsSize = 0;
    ValueVector mLocalsRegister;

    // -----
    std::unordered_map<uint32_t, size_t> mGlobalsLookUp;
    ValueVector mGlobalsRegister;

    //TODO must be initialized and on local push pop updated! Value* mVariableBases[2];

    // ----
    // bad idea here ?!
    VM mVM;


public:

    Environment() {
        mLocalsRegister.resize(LOCALS_SIZE);
        pushLocalStack(0);
        mLocalVarCount.reserve(16);
    }
    ~Environment() {
    }

    inline void PrintRuntimeError(const std::string& message, const std::string& p2 = "", const std::string& p3 = ""){
        Tools::errorf("RunTime Error [ENV:%p]: %s %s %s", (void*) this, message.c_str(), p2.c_str(),p3.c_str());
    }

    inline VM& getVM() { return mVM; }


    // -------------------------------------------------------------------------
    // GlobalsRegister
    // -------------------------------------------------------------------------
    S64 getGlobalRegister(U32 symId);
    size_t reserveGlobal(U32 symId);

    inline void saveGlobal(size_t index, const Value& value) {
        #ifdef DLS_DEBUG
        assert (index < mGlobalsRegister.size() && "SAVE: GLOBALS REGISTER OUT OF BOUNDS");
        #endif
        mGlobalsRegister[index] = value;
    }

    inline Value& getGlobal(size_t index) {
        #ifdef DLS_DEBUG
        assert (index < mGlobalsRegister.size() && "LOAD: GLOBALS REGISTER OUT OF BOUNDS");
        #endif
        return mGlobalsRegister[index];
    }

    bool getGlobalVariableValueRef(const std::string& varName, Value& ret);
    void printGlobals();
    // -------------------------------------------------------------------------
    // LocalRegister
    // -------------------------------------------------------------------------
    S64 getLocalsRootRegister(U32 symId);
    size_t reserveRootLocal(U32 symId);
    inline U32 getLocalsFrameId() {return  mLocalsFrameId;}

    inline void pushLocalStack(size_t varCount) {
        #ifdef DREIZEHN_DEBUG
        // Tools::printf("... pushLocalStack: %zu ....\n", varCount);
        #endif
        //TODO ROOT_LOCALS_OFFSET need to change the mLocalsSize
        mLocalOffset = mLocalsSize;
        mLocalVarCount.push_back(varCount);

        #ifdef DREIZEHN_DEBUG
        size_t newSize = mLocalOffset  + varCount;
        if (newSize > mLocalsRegister.capacity()) [[unlikely]] {
            PrintRuntimeError("Interpreter Locals Stack Overflow! raise LOCALS_SIZE as compile flag");
            exit(1);
        }
        #endif
        mLocalsSize = mLocalOffset  + varCount;
        mLocalsFrameId++;
    }
    inline void popLocalStack() {
        #ifdef DREIZEHN_DEBUG
        // Tools::printf("... popLocalStack count: %zu\n", mLocalVarCount.back());
        #endif
        if (mLocalVarCount.size() < 2) return; //we do not pop the root!
        mLocalsSize -=  mLocalVarCount.back();
        mLocalVarCount.pop_back();

        //FIXME  popped variables must be checked against Object Assign!

        mLocalOffset -=  mLocalVarCount.back();
        //TEST to remove mLocalsSize before:  mLocalOffset = mLocalsSize - mLocalVarCount.back();
        mLocalsFrameId--;
    }

    inline void saveLocal(size_t index, const Value& value) {
        #ifdef DREIZEHN_DEBUG
        // Tools::printf("saveLocal [%zu]:%s\n", index, value.toString().c_str() );
            assert ((index + mLocalOffset) < mLocalsSize && "SAVE: LOCALS REGISTER OUT OF BOUNDS");
        #endif
        mLocalsRegister[(index + mLocalOffset)] = value;
    }

    inline Value& getLocal(size_t index) {
        #ifdef DREIZEHN_DEBUG
        if ((index + mLocalOffset) >= mLocalsSize){
            Tools::errorf("LOAD: LOCALS REGISTER OUT OF BOUNDS: [%zu|%zu] => %zu\n", index, mLocalOffset, mLocalsRegister.size());
            assert(true && "LOAD: LOCALS REGISTER OUT OF BOUNDS");
        }
        Value& value = mLocalsRegister[(index + mLocalOffset)];
        // Tools::printf("LoadLocal [%zu|%zu] SIZE:%zu [P:%p] VALUE:%s\n", index, mLocalOffset,mLocalsRegister.size()
        //        ,(void*)&mLocalsRegister[(index + mLocalOffset)] ,  value.toString().c_str() );
        return value;
        #endif
        return mLocalsRegister[(index + mLocalOffset)];
    }


    // yay this is slightly faster
    template<TokenType Op>
    Value doMath(Value a, Value b) {
        double target_a = a.isDouble() ? a.asFastDouble() : (a.isInt() ? static_cast<double>(a.asFastInt()) : 0.0);
        double target_b = b.isDouble() ? b.asFastDouble() : (b.isInt() ? static_cast<double>(b.asFastInt()) : 0.0);

        if constexpr (Op == TokenType::Plus) {
            uint64_t result_bits = std::bit_cast<uint64_t>(target_a + target_b);
            return Value(Unsafe{}, result_bits);
        }
        else if constexpr (Op == TokenType::Minus) {
            uint64_t result_bits = std::bit_cast<uint64_t>(target_a - target_b);
            return Value(Unsafe{}, result_bits);
        }
        else if constexpr (Op == TokenType::Mul) {
            uint64_t result_bits = std::bit_cast<uint64_t>(target_a * target_b);
            return Value(Unsafe{}, result_bits);
        }
        else if constexpr (Op == TokenType::Div) {
            if (target_b == 0.0) [[unlikely]] return NumberNullValue;
            uint64_t result_bits = std::bit_cast<uint64_t>(target_a / target_b);
            return Value(Unsafe{}, result_bits);
        }
    }


    // -------------------------------------------------------------------------
    // EXECUTE :D - currentEnv for function calls
    // second should be faster for cpu but issnt so maybe a mix of both ?
    // -------------------------------------------------------------------------
    inline FlowStatus execute(ASTNode* node, Environment& currentEnv) {
        if (!node) return FlowStatus(FlowSignal::None);

        #ifdef DREIZEHN_DEBUG
        if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
        #endif

        if (node->mFlags & NodeFlag_Expression) [[likely]] {
            auto* expr = static_cast<Expression*>(node);
            expr->evaluate(currentEnv);
        } else if (node->mFlags & NodeFlag_Flow) {
            auto* flowStmt = static_cast<FlowBaseStatement*>(node);
            return flowStmt->execute(currentEnv);
        }

        return FlowStatus(FlowSignal::None);
    }

// // #else // AST_IP_SMALL_EXECUTE
// //     inline FlowStatus execute(ASTNode* node, Environment& currentEnv) {
// //         if (!node) return FlowStatus(FlowSignal::None);
// //
// // #ifdef DREIZEHN_DEBUG
// //         if (Globals::gDumpStateNodes) Tools::printf("EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
// // #endif
// //         switch(node->mNodeType) {
// //             // --- Break Statement ---
// //             case NodeType::BreakStatement:
// //             {
// //                 auto* flowStmt = static_cast<BreakStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::ContinueStatement:
// //             {
// //                 auto* flowStmt = static_cast<ContinueStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::ReturnStatement:
// //             {
// //                 auto* flowStmt = static_cast<ReturnStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::ElIfStatement:
// //             case NodeType::IfStatement:
// //             {
// //                 auto* flowStmt = static_cast<IfStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::RangeStatement:
// //             {
// //                 auto* flowStmt = static_cast<ForRangeStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::ForEachStatement:
// //             {
// //                 auto* flowStmt = static_cast<ForEachStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::ForStatement:
// //             {
// //                 auto* flowStmt = static_cast<ForStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::WhileStatement:
// //             {
// //                 auto* flowStmt = static_cast<WhileStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             case NodeType::BlockStatement:
// //             {
// //                 auto* flowStmt = static_cast<BlockStatement*>(node);
// //                 return flowStmt->execute(currentEnv);
// //             }
// //             // --- Assign ---
// //             case NodeType::AssignStatement: {
// //                 if (auto* assign = static_cast<AssignStatement*>(node)) {
// //                     assign->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::AssignOPStatement:
// //             {
// //                 if (auto* assign = static_cast<AssignOPStatement*>(node)) {
// //                     assign->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             // -------------- OTHERS -------------------
// //             case NodeType::LiteralExpression: {
// //                 if (auto* expr = static_cast<LiteralExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::ValueExpression: {
// //                 if (auto* expr = static_cast<ValueExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::VariableExpression: {
// //                 if (auto* expr = static_cast<VariableExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::CallExpression: {
// //                 if (auto* expr = static_cast<CallExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::MethodExpression: {
// //                 if (auto* expr = static_cast<MethodExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::BinaryExpression: {
// //                 if (auto* expr = static_cast<BinaryExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::BinaryOpExpression: {
// //                 if (auto* expr = static_cast<BinaryOpExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::BinaryInlineExpression: {
// //                 if (auto* expr = static_cast<BinaryInlineExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::ObjectFieldExpression: {
// //                 if (auto* expr = static_cast<ObjectFieldExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::ArrayVariableExpression: {
// //                 if (auto* expr = static_cast<ArrayVariableExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //
// //             case NodeType::IncludeStatement: {
// //                 if (auto* expr = static_cast<IncludeStatement*>(node)) {
// //                     return expr->execute(currentEnv);
// //                 }
// //                 break;
// //             }
// //
// //             case NodeType::KeyValueExpression: {
// //                 if (auto* expr = static_cast<KeyValueExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::StringConcatExpression: {
// //                 if (auto* expr = static_cast<StringConcatExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //             case NodeType::StringConcatAssignExpression: {
// //                 if (auto* expr = static_cast<StringConcatAssignExpression*>(node)) {
// //                     expr->evaluate(currentEnv);
// //                 }
// //                 break;
// //             }
// //
// //             // --- others ---
// //             default: {
// //                 // if (auto* expr = static_cast<Expression*>(node)) {
// //                 //     expr->evaluate(currentEnv);
// //                 // }
// //                 Tools::errorf("UNKNOWN Expression EXECUTE: %s\n", NodeTypeToString(node->mNodeType));
// //                 break;
// //             }
// //
// //         } // ... SWITCH ...
// //
// //         return FlowStatus(FlowSignal::None);
// //     }
// // #endif

    // -------------------------------------------------------------------------
    // main execute
    inline FlowStatus execute(ASTNode* node) {
         return execute(node, *this);
    }
    // -------------------------------------------------------------------------
}; //Class
} //Namespace
